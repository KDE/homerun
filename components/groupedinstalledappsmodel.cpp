/*
 * Copyright 2012 Aurélien Gâteau <agateau@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
// Self
#include <groupedinstalledappsmodel.h>

// Local
#include <abstractsourceregistry.h>
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
    return new InstalledAppsModel(group->entryPath(), m_installer, this);
}

//- GroupedInstalledAppsSource --------------------------------------
GroupedInstalledAppsSource::GroupedInstalledAppsSource(QObject *parent)
: AbstractSource(parent)
{}

QAbstractItemModel *GroupedInstalledAppsSource::createModelFromConfigGroup(const KConfigGroup &)
{
    KConfigGroup group(config(), "PackageManagement");
    QString installer = group.readEntry("categoryInstaller");
    return new GroupedInstalledAppsModel(installer);
}

} // namespace Homerun

#include <groupedinstalledappsmodel.moc>
