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
#include <groupedservicemodel.h>

// Local
#include <servicemodel.h>
#include <sourceid.h>
#include <sourceregistry.h>

// KDE
#include <KDebug>

// Qt

GroupedServiceModel::GroupedServiceModel(QObject *parent)
: QAbstractListModel(parent)
{
}

GroupedServiceModel::~GroupedServiceModel()
{
}

QObject *GroupedServiceModel::modelForRow(int row) const
{
    if (row < 0 || row >= m_models.count()) {
        kWarning() << "No model for row" << row << "!";
        return 0;
    }
    return m_models.value(row);
}

void GroupedServiceModel::loadRootEntries()
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

void GroupedServiceModel::loadNextGroup()
{
    if (m_pendingGroupList.isEmpty()) {
        return;
    }
    KServiceGroup::Ptr group = m_pendingGroupList.takeFirst();
    ServiceModel *model = createServiceModel(group);
    beginInsertRows(QModelIndex(), m_models.count(), m_models.count());
    m_models << model;
    endInsertRows();
    QMetaObject::invokeMethod(this, "loadNextGroup", Qt::QueuedConnection);
}

int GroupedServiceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_models.count();
}

QVariant GroupedServiceModel::data(const QModelIndex &index, int role) const
{
    if (index.parent().isValid()) {
        return QVariant();
    }

    ServiceModel *model = m_models.value(index.row());
    if (!model) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return model->name();
    } else {
        return QVariant();
    }
}

void GroupedServiceModel::init(SourceRegistry *registry)
{
    m_registry = registry;
    loadRootEntries();
}

ServiceModel *GroupedServiceModel::createServiceModel(KServiceGroup::Ptr group)
{
    Q_ASSERT(m_registry);
    SourceId sourceId;
    sourceId.setName("Service");
    sourceId.arguments().add("entryPath", group->entryPath());
    QObject *model = m_registry->createModelForSource(sourceId.toString(), this);
    return static_cast<ServiceModel *>(model);
}

//- GroupedServiceSource --------------------------------------
GroupedServiceSource::GroupedServiceSource(SourceRegistry *registry)
: AbstractSource(registry)
{}

QAbstractItemModel *GroupedServiceSource::createModel(const SourceArguments &/*args*/)
{
    GroupedServiceModel *model = new GroupedServiceModel;
    model->init(registry());
    return model;
}

#include <groupedservicemodel.moc>
