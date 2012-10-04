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

    bool setName(const QString &value)
    {
        if (m_name == value) {
            return false;
        }
        m_name = value;
        m_group.writeEntry("name", value);
        m_group.sync();
        return true;
    }

    static Tab *createFromGroup(const KConfigGroup &group)
    {
        // Read all mandatory keys first

        // (read "name" as QByteArray because i18n() wants a char* as argument)
        QString name = i18n(group.readEntry("name", QByteArray()));
        if (name.isEmpty()) {
            kWarning() << "Missing 'name' key in tab group" << group.name();
            return 0;
        }
        QStringList sources = readSources(group);
        if (sources.isEmpty()) {
            kWarning() << "No source defined in tab group" << group.name();
            return 0;
        }

        // Create tab and read optional keys
        Tab *tab = new Tab;
        tab->m_group = group;
        tab->m_name = name;
        tab->m_sources = sources;
        tab->m_iconName = group.readEntry("icon");
        // We use "query" because it is automatically extracted as a
        // translatable string by l10n-kde4/scripts/createdesktopcontext.pl
        QByteArray placeHolder = group.readEntry("query", QByteArray());
        if (!placeHolder.isEmpty()) {
            tab->m_searchPlaceholder = i18n(placeHolder);
        }
        return tab;
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

void TabModel::setConfig(const KSharedConfig::Ptr &ptr)
{
    beginResetModel();
    m_config = ptr;
    qDeleteAll(m_tabList);
    m_tabList.clear();
    QStringList tabGroupList;
    Q_FOREACH(const QString &groupName, m_config->groupList()) {
        if (groupName.startsWith("Tab")) {
            tabGroupList << groupName;
        }
    }
    tabGroupList.sort();
    Q_FOREACH(const QString &groupName, tabGroupList) {
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
    } else {
        kWarning() << "Don't know how to handle role" << roleName;
        return;
    }

    QModelIndex idx = index(row, 0);
    dataChanged(idx, idx);
}

#include "tabmodel.moc"
