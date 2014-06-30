/*
Copyright 2012 Aurélien Gâteau <agateau@kde.org>
Copyright 2013 Eike Hein <hein@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
// Self
#include <filterableinstalledappsmodel.h>

// Local
#include <abstractsourceregistry.h>
#include <changenotifier.h>
#include <installedappsmodel.h>

// KDE
#include <KConfigGroup>
#include <KDebug>

// Qt

namespace Homerun {

InstalledAppsFilterModel::InstalledAppsFilterModel(const QString &entryPath, const QString &installer, QObject *parent)
: QSortFilterProxyModel(parent)
, m_installedAppsModel(new InstalledAppsModel(entryPath, installer, this))
, m_hidden(true)
{
    setSourceModel(m_installedAppsModel);
    setDynamicSortFilter(true);
    setFilterRole(InstalledAppsModel::CombinedNameRole);
    setFilterCaseSensitivity(Qt::CaseInsensitive);

    connect(this, SIGNAL(modelReset()), this, SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SIGNAL(countChanged()));

    connect(m_installedAppsModel, SIGNAL(applicationLaunched(QString)), this, SIGNAL(applicationLaunched(QString)));
}

InstalledAppsFilterModel::~InstalledAppsFilterModel()
{
}

QObject *InstalledAppsFilterModel::containment() const
{
    return m_installedAppsModel->containment();
}

void InstalledAppsFilterModel::setContainment(QObject *containment)
{
    m_installedAppsModel->setContainment(containment);
}

QString InstalledAppsFilterModel::name() const
{
    return m_installedAppsModel->name();
}

bool InstalledAppsFilterModel::hidden() const
{
    return m_hidden;
}

void InstalledAppsFilterModel::setHidden(bool hidden)
{
    if (hidden != m_hidden) {
        m_hidden = hidden;
        emit hiddenChanged();
    }
}

int InstalledAppsFilterModel::count() const
{
    return rowCount();
}

void InstalledAppsFilterModel::refresh(bool reload)
{
    m_installedAppsModel->refresh(reload);
}

bool InstalledAppsFilterModel::trigger(int row, const QString &actionId, const QVariant &actionArgument)
{
    const QModelIndex &idx = index(row, 0);
    const QModelIndex &sourceIndex = mapToSource(idx);
    return m_installedAppsModel->trigger(sourceIndex.row(), actionId, actionArgument);
}

SideBarModel::SideBarModel(FilterableInstalledAppsModel *parent)
: QSortFilterProxyModel(parent)
, m_sourceModel(parent)
, m_activeSourceRow(0)
{
    setSourceModel(m_sourceModel);
    setDynamicSortFilter(true);

    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::CheckStateRole, "checked");

    setRoleNames(roles);
}

SideBarModel::~SideBarModel()
{
}

QVariant SideBarModel::data(const QModelIndex& index, int role) const
{
    QModelIndex sourceIndex = mapToSource(index);

    if (role == Qt::CheckStateRole) {
        return (sourceIndex.row() == m_activeSourceRow);
    }

    return QSortFilterProxyModel::data(index, role);
}

void SideBarModel::trigger(int row)
{
    const QModelIndex &idx = index(row, 0);
    const QModelIndex &sourceIndex = mapToSource(idx);

    if (sourceIndex.row() != m_activeSourceRow) {
        const QModelIndex &oldActiveIndex = mapFromSource(m_sourceModel->index(m_activeSourceRow, 0));

        m_activeSourceRow = sourceIndex.row();

        InstalledAppsFilterModel* model = 0;

        for(int i = 0; i < m_sourceModel->rowCount(); ++i) {
            model = static_cast<InstalledAppsFilterModel *>(m_sourceModel->modelForRow(i));
            model->setHidden(i != m_activeSourceRow);
        }

        emit dataChanged(oldActiveIndex, oldActiveIndex);
        emit dataChanged(idx, idx);
        invalidateFilter();
    }
}

bool SideBarModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent)

    return (m_activeSourceRow == source_row) || static_cast<InstalledAppsFilterModel *>(m_sourceModel->modelForRow(source_row))->count();
}

FilterableInstalledAppsModel::FilterableInstalledAppsModel(const QString &installer, QObject *parent)
: QAbstractListModel(parent)
, m_installer(installer)
, m_sideBarModel(new SideBarModel(this))
{
    loadRootEntries();
}

FilterableInstalledAppsModel::~FilterableInstalledAppsModel()
{
}

int FilterableInstalledAppsModel::count() const
{
    return rowCount();
}

QObject *FilterableInstalledAppsModel::modelForRow(int row) const
{
    if (row < 0 || row >= m_models.count()) {
        kWarning() << "No model for row" << row << "!";
        return 0;
    }
    return m_models.value(row);
}

void FilterableInstalledAppsModel::refresh(bool reload)
{
    if (!reload) {
        foreach(InstalledAppsFilterModel* model, m_models) {
            model->refresh(false);
        }

        return;
    }

    beginResetModel();
    qDeleteAll(m_models);
    m_models.clear();
    m_pendingGroupList.clear();
    endResetModel();

    loadRootEntries();

    emit countChanged();
}

void FilterableInstalledAppsModel::loadRootEntries()
{
    InstalledAppsFilterModel *model = createInstalledAppsFilterModel(KServiceGroup::root());
    model->setHidden(false);
    model->setParent(this);
    beginInsertRows(QModelIndex(), m_models.count(), m_models.count());
    m_models << model;
    endInsertRows();

    KServiceGroup::Ptr group = KServiceGroup::root();
    KServiceGroup::List list = group->entries(false /* sorted: set to false as it does not seem to work */);

    QMap<QString, KServiceGroup::Ptr> groupMap;
    for( KServiceGroup::List::ConstIterator it = list.constBegin(); it != list.constEnd(); it++) {
        const KSycocaEntry::Ptr p = (*it);

        if (p->isType(KST_KServiceGroup)) {
            KServiceGroup::Ptr subGroup = KServiceGroup::Ptr::staticCast(p);

            if (!subGroup->noDisplay() && subGroup->childCount() > 0) {
                groupMap.insert(subGroup->caption().toLower(), subGroup);
            }
        }
    }
    m_pendingGroupList = groupMap.values();
    QMetaObject::invokeMethod(this, "loadNextGroup", Qt::QueuedConnection);
}

void FilterableInstalledAppsModel::loadNextGroup()
{
    if (m_pendingGroupList.isEmpty()) {
        return;
    }
    KServiceGroup::Ptr group = m_pendingGroupList.takeFirst();
    InstalledAppsFilterModel *model = createInstalledAppsFilterModel(group);
    model->setParent(this);
    beginInsertRows(QModelIndex(), m_models.count(), m_models.count());
    m_models << model;
    endInsertRows();
    QMetaObject::invokeMethod(this, "loadNextGroup");
}

int FilterableInstalledAppsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_models.count();
}

QVariant FilterableInstalledAppsModel::data(const QModelIndex &index, int role) const
{
    if (index.parent().isValid()) {
        return QVariant();
    }

    InstalledAppsFilterModel *model = m_models.value(index.row());
    if (!model) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return model->name();
    } else {
        return QVariant();
    }
}

SideBarModel* FilterableInstalledAppsModel::sideBarModel() const
{
    return m_sideBarModel;
}

QString FilterableInstalledAppsModel::currentQuery() const
{
    return m_query;
}

void FilterableInstalledAppsModel::scheduleQuery(const QString& query)
{
    emit queryChanged(query);
    m_sideBarModel->invalidateFilter();
}

InstalledAppsFilterModel *FilterableInstalledAppsModel::createInstalledAppsFilterModel(KServiceGroup::Ptr group)
{
    InstalledAppsFilterModel *model = new InstalledAppsFilterModel(group->entryPath(), m_installer, this);
    connect(this, SIGNAL(queryChanged(QString)), model, SLOT(setFilterFixedString(QString)));
    connect(model, SIGNAL(applicationLaunched(QString)), this, SIGNAL(applicationLaunched(QString)));
    return model;
}

//- FilterableInstalledAppsSource --------------------------------------
FilterableInstalledAppsSource::FilterableInstalledAppsSource(QObject *parent)
: AbstractSource(parent)
{}

QAbstractItemModel *FilterableInstalledAppsSource::createModelFromConfigGroup(const KConfigGroup &)
{
    KConfigGroup group(config(), "PackageManagement");
    QString installer = group.readEntry("categoryInstaller");
    FilterableInstalledAppsModel *model = new FilterableInstalledAppsModel(installer);
    ChangeNotifier *notifier = new ChangeNotifier(model);
    connect(notifier, SIGNAL(changeDetected(bool)), model, SLOT(refresh(bool)));
    return model;
}

} // namespace Homerun

#include <filterableinstalledappsmodel.moc>
