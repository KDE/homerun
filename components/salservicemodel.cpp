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

SalServiceModel::SalServiceModel (QObject *parent)
    : QAbstractListModel(parent)
    , m_path("/")
{
    setPath("/");

    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "label");
    roles.insert(Qt::DecorationRole, "icon");
    roles.insert(EntryPathRole, "entryPath");

    setRoleNames(roles);
}

int SalServiceModel::rowCount(const QModelIndex& index) const
{
    return index.isValid() ? 0 : m_serviceList.count();
}

int SalServiceModel::count() const
{
    return m_serviceList.count();
}

QVariant SalServiceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_serviceList.count()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return m_serviceList.at(index.row())->name();
    } else if (role == Qt::DecorationRole) {
        return m_serviceList.at(index.row())->icon();
    } else if (role == EntryPathRole) {
        if (m_path == "/") {
            // Items at root level are not "favoritable", so don't return an entryPath
            return QVariant();
        }
        return m_serviceList.at(index.row())->entryPath();
    }

    return QVariant();
}

bool SalServiceModel::trigger(int row)
{
    KService::Ptr service = m_serviceList.at(row);
    if (m_path == "/") {
        // We are at root level, we want to descend into the selection, not execute
        QString salUrl = service->property("X-Plasma-Sal-Url").toString();
        // salUrl is of the form "kservicegroup://root/Something/". We want the "/Something" part.
        setPath("/" % salUrl.section('/', 2, -1, QString::SectionSkipEmpty));
        return false;
    } else {
        return KRun::run(*service, KUrl::List(), 0);
    }
}

void SalServiceModel::setPath(const QString &path)
{
    beginResetModel();
    m_path = path;
    m_serviceList.clear();

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
    QHash<QString, KServiceGroup::Ptr> groupSet;
    KServiceGroup::Ptr group = KServiceGroup::root();
    KServiceGroup::List list = group->entries();

    for( KServiceGroup::List::ConstIterator it = list.constBegin(); it != list.constEnd(); it++) {
        const KSycocaEntry::Ptr p = (*it);

        if (p->isType(KST_KServiceGroup)) {
            KServiceGroup::Ptr subGroup = KServiceGroup::Ptr::staticCast(p);

            if (!subGroup->noDisplay() && subGroup->childCount() > 0) {
                groupSet.insert(subGroup->relPath(), subGroup);
            }
        }
    }

    KService::List services = KServiceTypeTrader::self()->query("Plasma/Sal/Menu");
    if (!services.isEmpty()) {
        foreach (const KService::Ptr &service, services) {
            const QUrl url = QUrl(service->property("X-Plasma-Sal-Url", QVariant::String).toString());
            const QString groupName = url.path().remove(0, 1);

            if (url.scheme() != "kservicegroup" || groupSet.contains(groupName)) {
                m_serviceList.append(service);
            }

            if (groupSet.contains(groupName)) {
                groupSet.remove(groupName);
            }
        }
    }

    sort(-1, Qt::AscendingOrder);
}

void SalServiceModel::loadServiceGroup(KServiceGroup::Ptr group)
{
    if (group && group->isValid()) {
        KServiceGroup::List list = group->entries();

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
                    m_serviceList.append(service);
                }

            } else if (p->isType(KST_KServiceGroup)) {
                const KServiceGroup::Ptr subGroup = KServiceGroup::Ptr::staticCast(p);

                if (!subGroup->noDisplay() && subGroup->childCount() > 0) {
                    loadServiceGroup(subGroup);
                }
            }
        }
    }
    sort(0, Qt::AscendingOrder);
}

#include "salservicemodel.moc"
