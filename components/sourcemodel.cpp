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
#include <sourcemodel.h>

// Local
#include <abstractsourceregistry.h>
#include <customtypes.h>

// KDE
#include <KDebug>

// Qt

using namespace Homerun;

static const char *TAB_SOURCES_KEY = "sources";

static const char *SOURCE_GROUP_PREFIX = "Source";
static const char *SOURCE_SOURCEID_KEY = "sourceId";

class SourceModelItem
{
public:
    SourceModelItem(const QString &sourceId, QObject* model, const KConfigGroup &group)
    : m_sourceId(sourceId)
    , m_model(model)
    , m_group(group)
    {}

    QString m_sourceId;
    QObject *m_model;
    KConfigGroup m_group;
};


SourceModel::SourceModel(AbstractSourceRegistry *registry, const KConfigGroup &tabGroup, QObject *parent)
: QAbstractListModel(parent)
, m_sourceRegistry(registry)
, m_tabGroup(tabGroup)
{
    Q_ASSERT(registry);
    QHash<int, QByteArray> roles;
    roles.insert(SourceIdRole, "sourceId");
    roles.insert(ModelRole, "model");
    roles.insert(ConfigGroupRole, "configGroup");
    setRoleNames(roles);
    reload();
}

SourceModel::~SourceModel()
{
    qDeleteAll(m_list);
}

int SourceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_list.count();
}

QVariant SourceModel::data(const QModelIndex &index, int role) const
{
    SourceModelItem *item = m_list.value(index.row());
    if (!item) {
        kWarning() << "Invalid row" << index.row();
        return QVariant();
    }

    switch (role) {
    case SourceIdRole:
        return item->m_sourceId;
    case ModelRole:
        return QVariant::fromValue(item->m_model);
    case ConfigGroupRole:
        return QVariant::fromValue(&item->m_group);
    default:
        break;
    }
    return QVariant();
}

void SourceModel::reload()
{
    qDeleteAll(m_list);
    m_list.clear();

    QString tabGroupName = m_tabGroup.name();

    QStringList names = m_tabGroup.readEntry(TAB_SOURCES_KEY, QStringList());
    Q_FOREACH(const QString &name, names) {
        KConfigGroup sourceGroup(&m_tabGroup, name);
        QString sourceId = sourceGroup.readEntry(SOURCE_SOURCEID_KEY);

        QObject *model = m_sourceRegistry->createModelFromConfigGroup(sourceId, sourceGroup, this);
        if (!model) {
            continue;
        }
        SourceModelItem *item = new SourceModelItem(sourceId, model, sourceGroup);
        m_list << item;
    }
}

void SourceModel::appendSource(const QString &sourceId)
{
    KConfigGroup sourceGroup;
    for (int idx = 0; ; ++idx) {
        sourceGroup = KConfigGroup(&m_tabGroup, SOURCE_GROUP_PREFIX + QString::number(idx));
        if (!sourceGroup.exists()) {
            break;
        }
    }

    QObject *model = m_sourceRegistry->createModelFromConfigGroup(sourceId, sourceGroup, this);
    if (!model) {
        kWarning() << "Failed to create model for " << sourceId;
        return;
    }
    beginInsertRows(QModelIndex(), m_list.count(), m_list.count());
    SourceModelItem *item = new SourceModelItem(sourceId, model, sourceGroup);
    m_list << item;
    item->m_group.writeEntry(SOURCE_SOURCEID_KEY, sourceId);
    item->m_group.sync();
    writeSourcesEntry();
    endInsertRows();
}

void SourceModel::writeSourcesEntry()
{
    QStringList lst;
    Q_FOREACH(const SourceModelItem *item, m_list) {
        lst << item->m_group.name();
    }
    m_tabGroup.writeEntry(TAB_SOURCES_KEY, lst);
    m_tabGroup.sync();
}

void SourceModel::recreateModel(int row)
{
    SourceModelItem *item = m_list.value(row);
    if (!item) {
        kWarning() << "Invalid row" << row;
        return;
    }
    QObject *model = m_sourceRegistry->createModelFromConfigGroup(item->m_sourceId, item->m_group, this);
    if (!model) {
        kWarning() << "Failed to create model from row" << row;
        return;
    }
    delete item->m_model;
    item->m_model = model;

    QModelIndex idx = index(row, 0);
    dataChanged(idx, idx);
}

#define CHECK_ROW(row) \
    if (row < 0 || row >= m_list.count()) { \
        kWarning() << "Invalid row number" << row; \
        return; \
    }

void SourceModel::remove(int row)
{
    CHECK_ROW(row)
    beginRemoveRows(QModelIndex(), row, row);
    SourceModelItem *item = m_list.takeAt(row);
    item->m_group.deleteGroup();
    delete item;
    writeSourcesEntry();
    endRemoveRows();
}

void SourceModel::move(int from, int to)
{
    CHECK_ROW(from)
    CHECK_ROW(to)
    if (from == to) {
        kWarning() << "Cannot move row to itself";
        return;
    }
    // See beginMoveRows() doc for an explanation on modelTo
    int modelTo = to + (to > from ? 1 : 0);
    beginMoveRows(QModelIndex(), from, from, QModelIndex(), modelTo);
    m_list.move(from, to);
    writeSourcesEntry();
    endMoveRows();
}

#include <sourcemodel.moc>
