/*
    Copyright 2011 Aaron Seigo <aseigo@kde.org>

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
    : QStandardItemModel(parent)
    , m_path("/")
{
    QHash<int, QByteArray> newRoleNames = roleNames();
//    newRoleNames[CommonModel::Description] = "description";
//    newRoleNames[CommonModel::Url] = "url";
//    newRoleNames[CommonModel::Weight] = "weight";
//    newRoleNames[CommonModel::ActionTypeRole] = "action";

    setRoleNames(newRoleNames);

//   setPath("/");
    setPath("Internet/");

    //////////////////////////////////////////////////////////

    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "label");
    roles.insert(Qt::DecorationRole, "icon");
    roles.insert(Type, "type");
    roles.insert(Relevance, "relevance");
    roles.insert(Data, "data");
    roles.insert(Id, "id");
    roles.insert(SubText, "description");
    roles.insert(Enabled, "enabled");
    roles.insert(RunnerId, "runnerid");
    roles.insert(RunnerName, "runnerName");
    roles.insert(Actions, "actions");
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

void SalServiceModel::run(int index)
{
 //   if (index >= 0 && index < m_matches.count()) {
//        m_manager->run(m_matches.at(index));
  //  }
}

QVariant SalServiceModel::data(const QModelIndex &index, int role) const
{
//    if (!index.isValid() || index.parent().isValid() ||
//        index.column() > 0 || index.row() < 0 || index.row() >= m_matches.count()) {
//        // index requested must be valid, but we have no child items!
//        //kDebug() << "invalid index requested";
//        return QVariant();
//    }
//
    if (role == Qt::DisplayRole) {
        return m_serviceList.at(index.row())->name();
    } else if (role == Qt::DecorationRole) {
        return m_serviceList.at(index.row())->icon();
        return m_serviceList.at(index.row())->entryPath();
    } else if (role == Type) {
        return m_serviceList.at(index.row())->genericName();
 //       return m_matches.at(index.row()).type();
    } else if (role == Relevance) {
  //      return m_matches.at(index.row()).relevance();
    } else if (role == Data) {
//        return m_matches.at(index.row()).data();
    } else if (role == Id) {
  //      return m_matches.at(index.row()).id();
    } else if (role == SubText) {
    //    return m_matches.at(index.row()).subtext();
    } else if (role == Enabled) {
  //      return m_matches.at(index.row()).isEnabled();
    } else if (role == RunnerId) {
   //     return m_matches.at(index.row()).runner()->id();
    } else if (role == RunnerName) {
   //     return m_matches.at(index.row()).runner()->name();
    } else if (role == Actions) {
     //   QVariantList actions;
       // Plasma::QueryMatch amatch = m_matches.at(index.row());
      //  QList<QAction*> theactions = m_manager->actionsForMatch(amatch);
      // foreach(QAction* action, theactions) {
      //      actions += qVariantFromValue<QObject*>(action);
      //  }
       // return actions;
    }

    return QVariant();
}


bool SalServiceModel::openUrl(const KUrl& url)
{
    QString urlString = url.path();
    KService::Ptr service = KService::serviceByDesktopPath(urlString);

    if (!service) {
        service = KService::serviceByDesktopName(urlString);
    }

    if (!service) {
        return false;
    }

    return KRun::run(*service, KUrl::List(), 0);
}

QMimeData * SalServiceModel::mimeData(const QModelIndexList &indexes) const
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
        Q_ASSERT(KServiceGroup::group(path));
        loadServiceGroup(KServiceGroup::group(path));
        setSortRole(Qt::DisplayRole);
        sort(0, Qt::AscendingOrder);
    }

    endResetModel();
    emit countChanged();
}

QString SalServiceModel::path() const
{
    return m_path;
}

void SalServiceModel::loadRootEntries()
{
    QStringList defaultEnabledEntries;
    defaultEnabledEntries << "plasma-sal-contacts.desktop" << "plasma-sal-bookmarks.desktop"
    << "plasma-sal-multimedia.desktop" << "plasma-sal-internet.desktop"
    << "plasma-sal-graphics.desktop" << "plasma-sal-education.desktop"
    << "plasma-sal-games.desktop" << "plasma-sal-office.desktop";

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
                const int relevance = service->property("X-Plasma-Sal-Relevance", QVariant::Int).toInt();
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
                kDebug() << "$$$$$$$$$$$$$$$$$" << service->name() << "  COMMENT: " << service->comment() << "URL: " << url.toString();
                }

                    if (groupSet.contains(groupName)) {
                        groupSet.remove(groupName);
                    }
            }

//            m_serviceList.append(service);
        }

//        foreach (const KServiceGroup::Ptr group, groupSet) {
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
//        }
//
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
                    kDebug() << "LOADSERVICEGROUP %%%%%%%%:" << service->name() << service->entryPath() << genericName;
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
