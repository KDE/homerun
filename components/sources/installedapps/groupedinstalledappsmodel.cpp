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
#include <abstractsourceregistry.h>
#include <changenotifier.h>
#include <installedappsmodel.h>

// KDE
#include <KConfigGroup>
#include <KDebug>

// Qt

namespace Homerun {

GroupedInstalledAppsModel::GroupedInstalledAppsModel(const QString &installer, QObject *parent)
: QAbstractListModel(parent)
, m_installer(installer)
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
        foreach(InstalledAppsModel* model, m_models) {
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
    InstalledAppsModel *model = createInstalledAppsModel(group);
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

    InstalledAppsModel *model = m_models.value(index.row());
    if (!model) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return model->name();
    } else {
        return QVariant();
    }
}

InstalledAppsModel *GroupedInstalledAppsModel::createInstalledAppsModel(KServiceGroup::Ptr group)
{
    InstalledAppsModel *model = new InstalledAppsModel(group->entryPath(), m_installer, this);
    connect(model, SIGNAL(applicationLaunched(QString)), this, SIGNAL(applicationLaunched(QString)));
    return model;
}

//- GroupedInstalledAppsSource --------------------------------------
GroupedInstalledAppsSource::GroupedInstalledAppsSource(QObject *parent)
: AbstractSource(parent)
{}

QAbstractItemModel *GroupedInstalledAppsSource::createModelFromConfigGroup(const KConfigGroup &)
{
    KConfigGroup group(config(), "PackageManagement");
    QString installer = group.readEntry("categoryInstaller");
    GroupedInstalledAppsModel *model = new GroupedInstalledAppsModel(installer);
    ChangeNotifier *notifier = new ChangeNotifier(model);
    connect(notifier, SIGNAL(changeDetected(bool)), model, SLOT(refresh(bool)));
    return model;
}

} // namespace Homerun

#include <groupedinstalledappsmodel.moc>
