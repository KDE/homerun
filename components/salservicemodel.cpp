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
    kDebug() << "SALSERVICEMODEL INITED";
    setPath("/");

    //////////////////////////////////////////////////////////

    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "label");
    roles.insert(Qt::DecorationRole, "icon");
    roles.insert(Url, "url");

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
    } else if (role == Url) {
        QString pathAtRow = m_serviceList.at(index.row())->entryPath();

        // if we're at root level, we want to descend into the selection..not execute
        QString path;
        if (m_path == "/") {
            path = "kservicegroup:/" + pathAtRow;
            kDebug() << "PATH IS AT DATA:" << m_path << "RETURNING PATH OF: " << path;
        } else {
            path = pathAtRow;
        }
        return path;
    }

    return QVariant();
}

void SalServiceModel::run(int row)
{
    QModelIndex idx = index(row, 0);
    QString url = data(idx, Url).toString();
    openUrl(url);
}

bool SalServiceModel::openUrl(const QString& url)
{
    if (url.isEmpty()) {
        return false;
    }

    KService::Ptr service;

    if (url.startsWith("kservicegroup:/")) {
        //remove the kservicegroup:/ url thingy
//        const QString& trimmedUrl = url.right(url.length() - 15);
        service = KService::serviceByStorageId(url);
        QString salUrl = service->property("X-Plasma-Sal-Url").toString();
        // salUrl is of the form "kservicegroup://root/Something/". We want the "Something" part.
        setPath(salUrl.section('/', -1, -1, QString::SectionSkipEmpty) + '/');

        kDebug() << "SET PATH TO: " << m_path;

        return true;
    }

    // never reached if he navigated to a menu/submenu

    service  = KService::serviceByDesktopPath(url);
    kDebug() << "RETRIEVED SERVICE NAME FROM CLICKED: " << service->name();

    if (!service) {
        service = KService::serviceByDesktopName(url);
    }

    if (!service) {
        return false;
    }

    return KRun::run(*service, KUrl::List(), 0);
}

QMimeData * SalServiceModel::mimeData(const QModelIndexList &/*indexes*/) const
{
//    KUrl::List urls;
//
//    foreach (const QModelIndex & index, indexes) {
//        QString urlString = data(index, CommonModel::Url).toString();
//
//        KService::Ptr service = KService::serviceByDesktopPath(urlString);
//
//        if (!service) {
//            service = KService::serviceByDesktopName(urlString);
//        }
//
//        if (service) {
//            urls << KUrl(service->entryPath());
//        }
//    }
//
    QMimeData *mimeData = new QMimeData();

//    if (!urls.isEmpty()) {
//        urls.populateMimeData(mimeData);
//    }
//
    return mimeData;

}

void SalServiceModel::setPath(const QString &path)
{
    beginResetModel();
    m_path = path;
    m_serviceList.clear();

    if (path == "/") {
        loadRootEntries();
    } else {
        kDebug() << "TRYING TO SET PATH TO: " << path;
        loadServiceGroup(KServiceGroup::group(path));
 //       setSortRole(Qt::DisplayRole);
//        sort(0, Qt::AscendingOrder);
    }

    endResetModel();
    emit countChanged();
    pathChanged(path);
    kDebug() << "####### SETPATH CALLED (first time is from ctor)";
}

QString SalServiceModel::path() const
{
    return m_path;
}

void SalServiceModel::loadRootEntries()
{
//    QStringList defaultEnabledEntries;
//    defaultEnabledEntries << "plasma-sal-contacts.desktop" << "plasma-sal-bookmarks.desktop"
//    << "plasma-sal-multimedia.desktop" << "plasma-sal-internet.desktop"
//    << "plasma-sal-graphics.desktop" << "plasma-sal-education.desktop"
//    << "plasma-sal-games.desktop" << "plasma-sal-office.desktop";

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
//                    model->appendRow(
//                        StandardItemFactory::createItem(
//                            KIcon(service->icon()),
//                                                        service->name(),
//                                                        service->comment(),
//                                                        url.toString(),
//                                                        relevance,
//                                                        CommonModel::NoAction
//                        )
//                    );
                    m_serviceList.append(service);
                }

                if (groupSet.contains(groupName)) {
                    groupSet.remove(groupName);
                }
            }
        }

        foreach (const KServiceGroup::Ptr group, groupSet) {
//            if ((model != m_allRootEntriesModel)) {
//                model->appendRow(
//                    StandardItemFactory::createItem(
//                        KIcon(group->icon()),
//                                                    group->caption(),
//                                                    group->comment(),
//                                                    QString("kserviceGroup://root/") + group->relPath(),
//                                                    0.1,
//                                                    CommonModel::NoAction
//                    )
//                );
//            } else if (model == m_allRootEntriesModel) {
//                QStandardItem *item = StandardItemFactory::createItem(
//                    KIcon(group->icon()),
//                                                                      group->caption(),
//                                                                      group->comment(),
//                                                                      group->storageId(),
//                                                                      0.1,
//                                                                      CommonModel::NoAction
//                );
//                model->appendRow(item);
//            }
        }

//        model->setSortRole(CommonModel::Weight);
//        model->sort(0, Qt::DescendingOrder);
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
//                    appendRow(
//                        StandardItemFactory::createItem(
//                            KIcon(service->icon()),
//                                                        service->name(),
//                                                        genericName,
//                                                        service->entryPath(),
//                                                        0.5,
//                                                        CommonModel::AddAction
//                        )
//                    );
                }

            } else if (p->isType(KST_KServiceGroup)) {
                const KServiceGroup::Ptr subGroup = KServiceGroup::Ptr::staticCast(p);

                if (!subGroup->noDisplay() && subGroup->childCount() > 0) {
                    loadServiceGroup(subGroup);
                }
            }
        }
    }
}

#include "salservicemodel.moc"
