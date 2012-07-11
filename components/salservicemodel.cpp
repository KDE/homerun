/*
    Copyright 2011 Aaron Seigo <aseigo@kde.org>
    Copyright (C) 2012 Shaun Reich <shaun.reich@blue-systems.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "salservicemodel.h"

#include <QIcon>
#include <QAction>
#include <QTimer>

#include <KDebug>
#include <KService>
#include <KRun>
#include <KServiceTypeTrader>
#include <KSycocaEntry>

#include <Plasma/RunnerManager>

Node Node::fromServiceGroup(KServiceGroup::Ptr group)
{
    Node node;
    node.icon = KIcon(group->icon());
    node.name = group->caption();
    node.entryPath = group->entryPath();
    node.sortKey = node.name.toLower();
    return node;
}

Node Node::fromService(KService::Ptr service)
{
    Node node;
    node.icon = KIcon(service->icon());
    node.name = service->name();
    node.entryPath = service->entryPath();
    node.service = service;
    node.sortKey = node.name.toLower();
    return node;
}

bool Node::operator<(const Node &other) const
{
    return sortKey < other.sortKey;
}

SalServiceModel::SalServiceModel (QObject *parent)
    : QAbstractListModel(parent)
    , m_path("/")
{
    setPath("/");

    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "label");
    roles.insert(Qt::DecorationRole, "icon");
    roles.insert(EntryPathRole, "entryPath");
    roles.insert(FavoriteIdRole, "favoriteId");

    setRoleNames(roles);
}

int SalServiceModel::rowCount(const QModelIndex& index) const
{
    return index.isValid() ? 0 : m_nodeList.count();
}

int SalServiceModel::count() const
{
    return m_nodeList.count();
}

QVariant SalServiceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_nodeList.count()) {
        return QVariant();
    }

    const Node &node = m_nodeList.at(index.row());
    if (role == Qt::DisplayRole) {
        return node.name;
    } else if (role == Qt::DecorationRole) {
        return node.icon;
    } else if (role == EntryPathRole) {
        if (m_path == "/") {
            // Items at root level are not "favoritable", so don't return an entryPath
            return QVariant();
        }
        return node.entryPath;
    } else if (role == FavoriteIdRole) {
        if (m_path == "/") {
            return QString();
        } else {
            return QVariant(QString("app:") + node.service->storageId());
        }
    }

    return QVariant();
}

bool SalServiceModel::trigger(int row)
{
    const Node &node = m_nodeList.at(row);
    if (m_path == "/") {
        /*
        // We are at root level, we want to descend into the selection, not execute
        QString salUrl = service->property("X-Plasma-Sal-Url").toString();
        // salUrl is of the form "kservicegroup://root/Something/". We want the "/Something" part.
        setPath("/" % salUrl.section('/', 2, -1, QString::SectionSkipEmpty));
        */
        QString entryPath = node.entryPath;
        setPath("/" % entryPath.left(entryPath.length() - 1));
        return false;
    } else {
        return KRun::run(*node.service, KUrl::List(), 0);
    }
}

void SalServiceModel::setPath(const QString &path)
{
    beginResetModel();
    m_path = path;
    m_nodeList.clear();

    if (path == "/") {
        loadRootEntries();
    } else {
        QString relPath = path.mid(1) % "/";
        loadServiceGroup(KServiceGroup::group(relPath));
    }

    endResetModel();
    emit countChanged();
    pathChanged(path);
}

QString SalServiceModel::path() const
{
    return m_path;
}

void SalServiceModel::loadRootEntries()
{
    KServiceGroup::Ptr group = KServiceGroup::root();
    KServiceGroup::List list = group->entries(false /* sorted: set to false as it does not seem to work */);

    for( KServiceGroup::List::ConstIterator it = list.constBegin(); it != list.constEnd(); it++) {
        const KSycocaEntry::Ptr p = (*it);

        if (p->isType(KST_KServiceGroup)) {
            KServiceGroup::Ptr subGroup = KServiceGroup::Ptr::staticCast(p);

            if (!subGroup->noDisplay() && subGroup->childCount() > 0) {
                m_nodeList << Node::fromServiceGroup(subGroup);
            }
        }
    }
    qSort(m_nodeList);
}

void SalServiceModel::loadServiceGroup(KServiceGroup::Ptr group)
{
    if (group && group->isValid()) {
        KServiceGroup::List list = group->entries(false /* see above */);

        for( KServiceGroup::List::ConstIterator it = list.constBegin();
            it != list.constEnd(); it++) {
            const KSycocaEntry::Ptr p = (*it);

            if (p->isType(KST_KService)) {
                const KService::Ptr service = KService::Ptr::staticCast(p);

                if (!service->noDisplay()) {
                    QString genericName = service->genericName();
                    if (genericName.isNull()) {
                        genericName = service->comment();
                    }
                    m_nodeList << Node::fromService(service);
                }

            } else if (p->isType(KST_KServiceGroup)) {
                const KServiceGroup::Ptr subGroup = KServiceGroup::Ptr::staticCast(p);

                if (!subGroup->noDisplay() && subGroup->childCount() > 0) {
                    loadServiceGroup(subGroup);
                }
            }
        }
    }
    qSort(m_nodeList);
}

#include "salservicemodel.moc"
