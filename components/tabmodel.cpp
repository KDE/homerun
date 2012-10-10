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

static const char *SOURCE_KEY_PREFIX = "source";
static const char *TAB_GROUP_PREFIX = "Tab";

/**
 * Return values for all keys of a group which start with @p prefix
 */
static QStringList readSources(const KConfigGroup &group)
{
    QStringList lst;
    QMap<QString, QString> map = group.entryMap();
    auto it = map.constBegin(), end = map.constEnd();
    for (; it != end; ++it) {
        if (it.key().startsWith(SOURCE_KEY_PREFIX)) {
            lst << it.value();
        }
    }
    return lst;
}

class Tab
{
public:
    KConfigGroup m_group;

    QString m_name;
    QString m_iconName;
    QString m_searchPlaceholder;
    QStringList m_sources;

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

    void saveSources()
    {
        Q_FOREACH(const QString &key, m_group.keyList()) {
            if (key.startsWith(SOURCE_KEY_PREFIX)) {
                m_group.deleteEntry(key);
            }
        }
        int num = 0;
        Q_FOREACH(const QString &source, m_sources) {
            QString key = QLatin1String(SOURCE_KEY_PREFIX) + QString::number(num);
            m_group.writeEntry(key, source);
            ++num;
        }

        m_group.sync();
    }

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
        saveName();
        saveIconName();
        saveSources();
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
        tab->m_iconName = group.readEntry("icon");
        // We use "query" because it is automatically extracted as a
        // translatable string by l10n-kde4/scripts/createdesktopcontext.pl
        QByteArray placeHolder = group.readEntry("query", QByteArray());
        if (!placeHolder.isEmpty()) {
            tab->m_searchPlaceholder = i18n(placeHolder);
        }
        return tab;
    }

    void remove()
    {
        KConfig *config = m_group.config();
        m_group.deleteGroup();
        config->sync();
    }
};


TabModel::TabModel(QObject *parent)
: QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::DecorationRole, "decoration");
    roles.insert(SourcesRole, "sources");
    roles.insert(SearchPlaceholderRole, "searchPlaceholder");

    setRoleNames(roles);
}

TabModel::~TabModel()
{
    qDeleteAll(m_tabList);
}

QStringList TabModel::tabGroupList() const
{
    QStringList list;
    Q_FOREACH(const QString &groupName, m_config->groupList()) {
        if (groupName.startsWith(TAB_GROUP_PREFIX)) {
            list << groupName;
        }
    }
    list.sort();
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
    case SearchPlaceholderRole:
        return tab->m_searchPlaceholder;
    default:
        kWarning() << "Unhandled role" << role;
        return QVariant();
    }
}

void TabModel::setSourcesForRow(int row, const QVariant &value)
{
    Tab *tab = m_tabList.value(row);
    if (!tab) {
        kWarning() << "Invalid row number" << row;
        return;
    }
    QStringList sources = value.toStringList();
    tab->m_sources = sources;
    tab->saveSources();

    QModelIndex idx = index(row, 0);
    dataChanged(idx, idx);
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
    for (int row = 0; row < m_tabList.count(); ++row) {
        Tab *tab = m_tabList[row];
        tab->m_group = KConfigGroup(m_config, QLatin1String(TAB_GROUP_PREFIX) + QString::number(row));
        tab->save();
    }
    endMoveRows();
}

#include "tabmodel.moc"
