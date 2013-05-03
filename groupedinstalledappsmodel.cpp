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
#include <groupedinstalledappsmodel.h>

// Local
#include <changenotifier.h>
#include <installedappsmodel.h>

// KDE
#include <KConfigGroup>
#include <KDebug>

// Qt


namespace Homerun {

// First argument is the library name without the "homerun_source_" prefix
// Second argument is the class name of the source
HOMERUN_EXPORT_SOURCE(appswithfilters, GroupedInstalledAppsSource)

InstalledAppsFilterModel::InstalledAppsFilterModel(const QString &entryPath, const QString &installer, QObject *parent)
: QSortFilterProxyModel(parent)
, m_installedAppsModel(new InstalledAppsModel(entryPath, installer, this))
{
    setSourceModel(m_installedAppsModel);
    setDynamicSortFilter(true);
    setFilterCaseSensitivity(Qt::CaseInsensitive);

    connect(m_installedAppsModel, SIGNAL(modelReset()), this, SIGNAL(countChanged()));
    connect(m_installedAppsModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SIGNAL(countChanged()));
    connect(m_installedAppsModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SIGNAL(countChanged()));
}

InstalledAppsFilterModel::~InstalledAppsFilterModel()
{
}

QString InstalledAppsFilterModel::name() const
{
    return m_installedAppsModel->name();
}

int InstalledAppsFilterModel::count() const
{
    return rowCount();
}

void InstalledAppsFilterModel::refresh(bool reload)
{
    m_installedAppsModel->refresh(reload);
}

bool InstalledAppsFilterModel::trigger(int row)
{
    return m_installedAppsModel->trigger(row);
}

FilterModel::FilterModel(GroupedInstalledAppsModel *sourceModel)
: QSortFilterProxyModel(sourceModel)
, m_sourceModel(sourceModel)
, m_activeFilterIndex(0)
{
    setSourceModel(m_sourceModel);
    setDynamicSortFilter(true);

    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::DecorationRole, "decoration");
    roles.insert(ActiveFilterRole, "activeFilter");

    setRoleNames(roles);
}

FilterModel::~FilterModel()
{
}

QVariant FilterModel::data(const QModelIndex &index, int role) const
{
    if (index.parent().isValid()) {
        return QVariant();
    }

    if (role == ActiveFilterRole) {
        if (index.row() == m_activeFilterIndex) {
            return true;
        } else {
            return false;
        }
    }

    return QSortFilterProxyModel::data(index, role);
}

int FilterModel::activeFilterIndex() const
{
    return m_activeFilterIndex;
}

void FilterModel::setActiveFilterIndex(int index)
{
    if (index != m_activeFilterIndex) {
        int oldIndex = m_activeFilterIndex;
        m_activeFilterIndex = index;
        emit activeFilterIndexChanged();
        const QModelIndex &oldModelIndex = FilterModel::index(oldIndex, 0);
        emit dataChanged(oldModelIndex, oldModelIndex);
        const QModelIndex &modelIndex = FilterModel::index(index, 0);
        emit dataChanged(modelIndex, modelIndex);
    }
}

bool FilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent)

    return static_cast<InstalledAppsFilterModel*>(m_sourceModel->modelForRow(source_row))->count();
}

GroupedInstalledAppsModel::GroupedInstalledAppsModel(const QString &installer, QObject *parent)
: QAbstractListModel(parent)
, m_installer(installer)
, m_filterModel(new FilterModel(this))
{
    loadRootEntries();
}

GroupedInstalledAppsModel::~GroupedInstalledAppsModel()
{
}

QObject *GroupedInstalledAppsModel::modelForRow(int row) const
{
    if (row < 0 || row >= m_models.count()) {
        kWarning() << "No model for row" << row << "!";
        return 0;
    }
    return m_models.value(row);
}

void GroupedInstalledAppsModel::refresh(bool reload)
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
}

void GroupedInstalledAppsModel::loadRootEntries()
{
    InstalledAppsFilterModel *model = createInstalledAppsModel(KServiceGroup::root());
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

void GroupedInstalledAppsModel::loadNextGroup()
{
    if (m_pendingGroupList.isEmpty()) {
        return;
    }
    KServiceGroup::Ptr group = m_pendingGroupList.takeFirst();
    InstalledAppsFilterModel *model = createInstalledAppsModel(group);
    model->setParent(this);
    beginInsertRows(QModelIndex(), m_models.count(), m_models.count());
    m_models << model;
    endInsertRows();
    QMetaObject::invokeMethod(this, "loadNextGroup", Qt::QueuedConnection);
}

int GroupedInstalledAppsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_models.count();
}

QVariant GroupedInstalledAppsModel::data(const QModelIndex &index, int role) const
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

FilterModel* GroupedInstalledAppsModel::filterModel() const
{
    return m_filterModel;
}

QString GroupedInstalledAppsModel::currentQuery() const
{
    return m_query;
}

void GroupedInstalledAppsModel::scheduleQuery(const QString& query)
{
    emit queryChanged(query);
    m_filterModel->invalidateFilter();
}

InstalledAppsFilterModel *GroupedInstalledAppsModel::createInstalledAppsModel(KServiceGroup::Ptr group)
{
    InstalledAppsFilterModel *appsFilterModel = new InstalledAppsFilterModel(group->entryPath(), m_installer, this);
    connect(this, SIGNAL(queryChanged(QString)), appsFilterModel, SLOT(setFilterFixedString(QString)));
    return appsFilterModel;
}

//- GroupedInstalledAppsSource --------------------------------------
GroupedInstalledAppsSource::GroupedInstalledAppsSource(QObject *parent, const QVariantList &args)
: AbstractSource(parent, args)
{}

QAbstractItemModel *GroupedInstalledAppsSource::createModelFromConfigGroup(const KConfigGroup &)
{
    KConfigGroup group(config(), "PackageManagement");
    QString installer = group.readEntry("categoryInstaller");
    GroupedInstalledAppsModel *model = new GroupedInstalledAppsModel(installer);
    ChangeNotifier *notifier = new ChangeNotifier(model);
    connect(notifier, SIGNAL(changeDetected()), model, SLOT(refresh()));
    return model;
}

} // namespace Homerun

#include <groupedinstalledappsmodel.moc>
