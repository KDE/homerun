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
#include "pagemodel.h"

// KDE
#include <KConfigGroup>
#include <KDebug>
#include <KLocale>

/**
 * Return values for all keys of a group which start with @p prefix
 */
static QStringList readSources(const KConfigGroup &group, const QString &prefix)
{
    QStringList lst;
    QMap<QString, QString> map = group.entryMap();
    auto it = map.constBegin(), end = map.constEnd();
    for (; it != end; ++it) {
        if (it.key().startsWith(prefix)) {
            lst << it.value();
        }
    }
    return lst;
}

class Page
{
public:
    QString m_name;
    QString m_iconName;
    QString m_searchPlaceholder;
    QStringList m_sources;
    QStringList m_searchSources;

    static Page *createFromGroup(const KConfigGroup &group)
    {
        // Read all mandatory keys first

        // (read "name" as QByteArray because i18n() wants a char* as argument)
        QString name = i18n(group.readEntry("name", QByteArray()));
        if (name.isEmpty()) {
            kWarning() << "Missing 'name' key in page group" << group.name();
            return 0;
        }
        QStringList sources = readSources(group, "source");
        if (sources.isEmpty()) {
            kWarning() << "No source defined in page group" << group.name();
            return 0;
        }

        // Create page and read optional keys
        Page *page = new Page;
        page->m_name = name;
        page->m_sources = sources;
        page->m_searchSources = readSources(group, "searchSource");
        page->m_iconName = group.readEntry("icon");
        // We use "query" because it is automatically extracted as a
        // translatable string by l10n-kde4/scripts/createdesktopcontext.pl
        QByteArray placeHolder = group.readEntry("query", QByteArray());
        if (!placeHolder.isEmpty()) {
            page->m_searchPlaceholder = i18n(placeHolder);
        }
        return page;
    }
};


PageModel::PageModel(QObject *parent)
: QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "name");
    roles.insert(IconNameRole, "iconName");
    roles.insert(SourcesRole, "sources");
    roles.insert(SearchSourcesRole, "searchSources");
    roles.insert(SearchPlaceholderRole, "searchPlaceholder");

    setRoleNames(roles);
}

PageModel::~PageModel()
{
    qDeleteAll(m_pageList);
}

void PageModel::setConfig(const KSharedConfig::Ptr &ptr)
{
    beginResetModel();
    m_config = ptr;
    qDeleteAll(m_pageList);
    m_pageList.clear();
    QStringList pageGroupList;
    Q_FOREACH(const QString &groupName, m_config->groupList()) {
        if (groupName.startsWith("Page")) {
            pageGroupList << groupName;
        }
    }
    pageGroupList.sort();
    Q_FOREACH(const QString &groupName, pageGroupList) {
        KConfigGroup group = m_config->group(groupName);
        Page *page = Page::createFromGroup(group);
        if (page) {
            m_pageList << page;
        }
    }
    endResetModel();
    configFileNameChanged(m_config->name());
}

QString PageModel::configFileName() const
{
    return m_config ? m_config->name() : QString();
}

void PageModel::setConfigFileName(const QString &name)
{
    if (name == configFileName()) {
        return;
    }
    setConfig(KSharedConfig::openConfig(name));
}

int PageModel::rowCount(const QModelIndex &parent) const
{
    if (m_config.isNull()) {
        return 0;
    }
    if (parent.isValid()) {
        return 0;
    }
    return m_pageList.count();
}

QVariant PageModel::data(const QModelIndex &index, int role) const
{
    Page *page = m_pageList.value(index.row());
    if (!page) {
        return QVariant();
    }
    switch (role) {
    case Qt::DisplayRole:
        return page->m_name;
    case IconNameRole:
        return page->m_iconName;
    case SourcesRole:
        return page->m_sources;
    case SearchSourcesRole:
        return page->m_searchSources;
    case SearchPlaceholderRole:
        return page->m_searchPlaceholder;
    default:
        kWarning() << "Unhandled role" << role;
        return QVariant();
    }
}

#include "pagemodel.moc"
