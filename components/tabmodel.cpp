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
#include "tabmodel.h"

// KDE
#include <KConfigGroup>
#include <KDebug>
#include <KLocale>

// Local
#include <sourceid.h>

#define MIGRATE_V1_CONFIG_FILE_FORMAT

using namespace Homerun;

static const char *TAB_GROUP_PREFIX = "Tab";
static const char *TAB_SOURCES_KEY = "sources";

static const char *SOURCE_GROUP_PREFIX = "Source";
static const char *SOURCE_SOURCEID_KEY = "sourceId";

#ifdef MIGRATE_V1_CONFIG_FILE_FORMAT
static const char *TAB_V1_SOURCE_KEY_PREFIX = "source";
#endif

#ifdef MIGRATE_V1_CONFIG_FILE_FORMAT
static QStringList takeLegacySources(const KConfigGroup &group_)
{
    QStringList sources;

    KConfigGroup group(group_);

    QMap<QString, QString> map = group.entryMap();
    auto it = map.constBegin(), end = map.constEnd();
    for (; it != end; ++it) {
        if (!it.key().startsWith(TAB_V1_SOURCE_KEY_PREFIX)) {
            continue;
        }

        sources << it.value();
        group.deleteEntry(it.key());
    }
    return sources;
}
#endif

/**
 * Returns a QList<sourceList>
 * Where sourceList is a QList<"config", $sourceId, QList<$groupNames> >
 */
QVariantList readSources(const KConfigGroup &tabGroup)
{
    QStringList names = tabGroup.readEntry(TAB_SOURCES_KEY, QStringList());
    QVariantList lst;

    QString tabGroupName = tabGroup.name();

    Q_FOREACH(const QString &name, names) {
        KConfigGroup sourceGroup(&tabGroup, name);
        QString sourceId = sourceGroup.readEntry(SOURCE_SOURCEID_KEY);

        QVariantList sourceList;
        sourceList.append(QLatin1String("config"));
        sourceList.append(sourceId);
        sourceList.append(QStringList() << tabGroupName << name);
        lst.append(QVariant(sourceList));
    }
    return lst;
}

class Tab
{
public:
    KConfigGroup m_group;

    QString m_name;
    QString m_iconName;
    QVariantList m_sources;

    bool setName(const QString &value)
    {
        if (m_name == value) {
            return false;
        }
        m_name = value;
        saveName();
        m_group.sync();
        return true;
    }

    bool setIconName(const QString &value)
    {
        if (m_iconName == value) {
            return false;
        }
        m_iconName = value;
        saveIconName();
        m_group.sync();
        return true;
    }

#ifdef MIGRATE_V1_CONFIG_FILE_FORMAT
    void saveSources(const QStringList &sourceIds)
    {
        // Delete all source groups
        Q_FOREACH(const QString &name, m_group.groupList()) {
            if (name.startsWith(SOURCE_GROUP_PREFIX)) {
                KConfigGroup(&m_group, name).deleteGroup();
            }
        }

        // Write new source groups
        QStringList sourceGroupNames;
        int num = 0;
        Q_FOREACH(const QString &source, sourceIds) {
            bool ok;
            QString groupName = QLatin1String(SOURCE_GROUP_PREFIX) + QString::number(num);
            SourceId sourceId = SourceId::fromString(source, &ok);
            Q_ASSERT(ok);

            KConfigGroup sourceGroup(&m_group, groupName);
            sourceGroup.writeEntry(SOURCE_SOURCEID_KEY, sourceId.name());

            const SourceArguments &args = sourceId.arguments();
            auto it = args.constBegin(), end = args.constEnd();
            for (; it != end; ++it) {
                sourceGroup.writeEntry(it.key(), it.value());
            }

            ++num;
            sourceGroupNames << groupName;
        }

        m_group.writeEntry(TAB_SOURCES_KEY, sourceGroupNames);
    }
#endif

    void saveName()
    {
        m_group.writeEntry("name", m_name);
    }

    void saveIconName()
    {
        m_group.writeEntry("icon", m_iconName);
    }

    void save()
    {
        // In case a deleted group is reused
        m_group.writeEntry("deleted", false);
        saveName();
        saveIconName();
        m_group.sync();
    }

    static Tab *createFromGroup(const KConfigGroup &group)
    {
        Tab *tab = new Tab;

        // (read "name" as QByteArray because i18n() wants a char* as argument)
        QByteArray name = group.readEntry("name", QByteArray());
        if (!name.isEmpty()) {
            // Only translate if not empty. If name is empty i18n(name)
            // returns I18N_EMPTY_MESSAGE.
            tab->m_name = i18n(name);
        }

        tab->m_group = group;
        tab->m_sources = readSources(group);
#ifdef MIGRATE_V1_CONFIG_FILE_FORMAT
        if (tab->m_sources.isEmpty()) {
            QStringList sourceIds = takeLegacySources(group);
            tab->saveSources(sourceIds);
            tab->m_group.sync();
            tab->m_sources = readSources(group);
        }
#endif
        tab->m_iconName = group.readEntry("icon");
        return tab;
    }

    void remove()
    {
        m_group.deleteGroup();
        m_group.writeEntry("deleted", true);
        m_group.sync();
    }
};


TabModel::TabModel(QObject *parent)
: QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::DecorationRole, "decoration");
    roles.insert(SourcesRole, "sources");

    setRoleNames(roles);
}

TabModel::~TabModel()
{
    qDeleteAll(m_tabList);
}

QStringList TabModel::tabGroupList() const
{
    KConfigGroup group (m_config, "General");
    QStringList list = group.readEntry("tabs", QStringList());
#ifdef MIGRATE_V1_CONFIG_FILE_FORMAT
    if (!list.isEmpty()) {
        return list;
    }

    // Create "tabs" key if it does not exist
    Q_FOREACH(const QString &groupName, m_config->groupList()) {
        if (groupName.startsWith(TAB_GROUP_PREFIX)) {
            KConfigGroup group = m_config->group(groupName);
            if (group.readEntry("deleted", false)) {
                continue;
            }
            list << groupName;
        }
    }
    list.sort();
    group.writeEntry("tabs", list);
    const_cast<TabModel *>(this)->m_config->sync();
#endif

    return list;
}

void TabModel::setConfig(const KSharedConfig::Ptr &ptr)
{
    beginResetModel();
    m_config = ptr;
    qDeleteAll(m_tabList);
    m_tabList.clear();
    QStringList list = tabGroupList();
    Q_FOREACH(const QString &groupName, list) {
        KConfigGroup group = m_config->group(groupName);
        Tab *tab = Tab::createFromGroup(group);
        if (tab) {
            m_tabList << tab;
        }
    }
    endResetModel();
    configFileNameChanged(m_config->name());
}

QString TabModel::configFileName() const
{
    return m_config ? m_config->name() : QString();
}

void TabModel::setConfigFileName(const QString &name)
{
    if (name == configFileName()) {
        return;
    }
    setConfig(KSharedConfig::openConfig(name));
}

int TabModel::rowCount(const QModelIndex &parent) const
{
    if (m_config.isNull()) {
        return 0;
    }
    if (parent.isValid()) {
        return 0;
    }
    return m_tabList.count();
}

QVariant TabModel::data(const QModelIndex &index, int role) const
{
    Tab *tab = m_tabList.value(index.row());
    if (!tab) {
        return QVariant();
    }
    switch (role) {
    case Qt::DisplayRole:
        return tab->m_name;
    case Qt::DecorationRole:
        return tab->m_iconName;
    case SourcesRole:
        return tab->m_sources;
    default:
        kWarning() << "Unhandled role" << role;
        return QVariant();
    }
}

void TabModel::setSourcesForRow(int row, const QVariant &value)
{
    /*
    Tab *tab = m_tabList.value(row);
    if (!tab) {
        kWarning() << "Invalid row number" << row;
        return;
    }
    QStringList sources = value.toStringList();
    tab->m_sources = sources;

    QModelIndex idx = index(row, 0);
    dataChanged(idx, idx);
    */
}

void TabModel::setDataForRow(int row, const QByteArray &roleName, const QVariant &value)
{
    Tab *tab = m_tabList.value(row);
    if (!tab) {
        kWarning() << "Invalid row number" << row;
        return;
    }

    if (roleName == "display") {
        if (!tab->setName(value.toString())) {
            return;
        }
    } else if (roleName == "decoration") {
        if (!tab->setIconName(value.toString())) {
            return;
        }
    } else {
        kWarning() << "Don't know how to handle role" << roleName;
        return;
    }

    QModelIndex idx = index(row, 0);
    dataChanged(idx, idx);
}

void TabModel::appendRow()
{
    QStringList list = tabGroupList();
    bool ok;
    int lastId = list.last().mid(3).toInt(&ok);
    if (!ok) {
        kWarning() << "Cannot extract a valid lastId from" << list.last();
        return;
    }

    Tab *tab = new Tab;
    tab->m_group = m_config->group(QLatin1String(TAB_GROUP_PREFIX) + QString::number(lastId + 1));

    int count = m_tabList.count();
    beginInsertRows(QModelIndex(), count, count);
    m_tabList.append(tab);
    endInsertRows();

    tab->save();
    writeGeneralTabsEntry();
}

#define CHECK_ROW(row) \
    if (row < 0 || row >= m_tabList.count()) { \
        kWarning() << "Invalid row number" << row; \
        return; \
    }

void TabModel::removeRow(int row)
{
    CHECK_ROW(row)
    beginRemoveRows(QModelIndex(), row, row);
    Tab *tab = m_tabList.takeAt(row);
    Q_ASSERT(tab);
    tab->remove();
    delete tab;
    writeGeneralTabsEntry();
    endRemoveRows();
}

void TabModel::moveRow(int from, int to)
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
    m_tabList.move(from, to);
    writeGeneralTabsEntry();
    endMoveRows();
}

void TabModel::writeGeneralTabsEntry()
{
    QStringList lst;
    Q_FOREACH(const Tab *tab, m_tabList) {
        lst << tab->m_group.name();
    }
    KConfigGroup group(m_config, "General");
    group.writeEntry("tabs", lst);
    m_config->sync();
}

#include "tabmodel.moc"
