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

// KDE
#include <KDebug>

// Qt

using namespace Homerun;

static const char *TAB_SOURCES_KEY = "sources";

static const char *SOURCE_SOURCEID_KEY = "sourceId";

class SourceModelItem
{
public:
    SourceModelItem(const QString &sourceId, QObject* model, const KConfigGroup &group)
    : m_sourceId(sourceId)
    , m_model(model)
    , m_configGroup(group)
    {}

    QString m_sourceId;
    QObject *m_model;
    KConfigGroup m_configGroup;
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
        return qVariantFromValue(item->m_model);
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

#include <sourcemodel.moc>
