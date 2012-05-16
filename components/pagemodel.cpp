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

#include <KConfigGroup>
#include <KDebug>

class Page
{
public:
    QString m_name;
    QString m_iconName;
    QString m_modelName;
    QStringList m_modelArgs;

    static Page *createFromGroup(const KConfigGroup &group)
    {
        // Read all mandatory keys first
        QString name = group.readEntry("name");
        if (name.isEmpty()) {
            kWarning() << "Missing 'name' key in page group" << group.name();
            return 0;
        }
        QString modelName = group.readEntry("modelName");
        if (modelName.isEmpty()) {
            kWarning() << "Missing 'modelName' key in page group" << group.name();
            return 0;
        }

        // Create page and read optional keys
        Page *page = new Page;
        page->m_name = name;
        page->m_modelName = modelName;
        page->m_iconName = group.readEntry("icon");
        page->m_modelArgs = group.readEntry("modelArgs", QStringList());
        return page;
    }
};


PageModel::PageModel(QObject *parent)
: QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "name");
    roles.insert(IconNameRole, "iconName");
    roles.insert(ModelNameRole, "modelName");
    roles.insert(ModelArgsRole, "modelArgs");

    setRoleNames(roles);
}

PageModel::~PageModel()
{
    qDeleteAll(m_pageList);
}

QString PageModel::configFileName() const
{
    return m_config.isNull() ? QString() : m_config->name();
}

void PageModel::setConfigFileName(const QString &name)
{
    if (name == configFileName()) {
        return;
    }
    setConfig(KSharedConfig::openConfig(name));
}

void PageModel::setConfig(const KSharedConfig::Ptr &ptr)
{
    beginResetModel();
    m_config = ptr;
    qDeleteAll(m_pageList);
    m_pageList.clear();
    Q_FOREACH(const QString &groupName, m_config->groupList()) {
        if (!groupName.startsWith("Page ")) {
            continue;
        }
        KConfigGroup group = m_config->group(groupName);
        Page *page = Page::createFromGroup(group);
        if (page) {
            m_pageList << page;
        }
    }
    endResetModel();

    configFileNameChanged();
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
    case ModelNameRole:
        return page->m_modelName;
    case ModelArgsRole:
        return page->m_modelArgs;
    default:
        kWarning() << "Unhandled role" << role;
        return QVariant();
    }
}

#include "pagemodel.moc"
