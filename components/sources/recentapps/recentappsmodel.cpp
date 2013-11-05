/*
Copyright 2013 Eike Hein <hein@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
// Local
#include <actionlist.h>
#include <recentappsmodel.h>
#include <sourceregistry.h>

// Qt
#include <QApplication>

// KDE
#include <KRun>
#include <KService>

#include <Plasma/Containment>
#include <Plasma/Corona>

namespace Homerun {

//- RecentAppsModel ------------------------------------------------------------
RecentAppsModel::RecentAppsModel(const KConfigGroup &group, QObject *parent)
: QAbstractListModel(parent)
, m_configGroup(group)
, m_containment(0)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::DecorationRole, "decoration");
    roles.insert(FavoriteIdRole, "favoriteId");
    roles.insert(HasActionListRole, "hasActionList");
    roles.insert(ActionListRole, "actionList");

    setRoleNames(roles);

    QList<QString> apps = group.readEntry("RecentApps", QList<QString>());

    if (apps.isEmpty()) {
        addApp("systemsettings.desktop", false);
    } else {
        for (int i = apps.count() - 1; i >= 0; --i) {
            addApp(apps.at(i), false);
        }
    }
}

RecentAppsModel::~RecentAppsModel()
{
}

QObject *RecentAppsModel::containment() const
{
    return m_containment;
}

void RecentAppsModel::setContainment(QObject *containment)
{
    m_containment = containment;
}

int RecentAppsModel::count() const
{
    return m_storageIdList.count();
}

int RecentAppsModel::rowCount(const QModelIndex& index) const
{
    return index.isValid() ? 0 : m_storageIdList.count();
}

QVariant RecentAppsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_storageIdList.count()) {
        return QVariant();
    }

    const QString storageId = m_storageIdList.at(index.row());
    KService::Ptr service = KService::serviceByStorageId(storageId);

    if (!service) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return service->name();
    } else if (role == Qt::DecorationRole) {
        // at least show the oxygen question-mark, otherwise it looks weird blank.
        return service->icon().isEmpty() ? QLatin1String("unknown") : service->icon();
    } else if (role == FavoriteIdRole) {
        return QVariant(QString("app:") + storageId);
    } else if (role == HasActionListRole) {
        return true;
    } else if (role == ActionListRole) {
        QVariantList actionList;

        QVariantMap forgetAction = Homerun::ActionList::createActionItem(i18n("Forget Application"), "forget");
        actionList.append(forgetAction);

        actionList.append(Homerun::ActionList::createSeparatorActionItem());

        if (qApp->property("HomerunViewerAdaptor").isValid())
        {
            if (qApp->property("desktopContainmentId").toUInt() > 0
                && qApp->property("desktopContainmentMutable").toBool()) {
                actionList << ActionList::createActionItem(i18n("Add to Desktop"), "addToDesktop");
            }
            if (qApp->property("appletContainmentId").toUInt() > 0
                && qApp->property("appletContainmentMutable").toBool()) {
                actionList << ActionList::createActionItem(i18n("Add to Panel"), "addToPanel");
            }
        } else if (m_containment) {
            Plasma::Containment *containment = static_cast<Plasma::Containment *>(m_containment);
            Plasma::Containment *desktop = containment->corona()->containmentForScreen(containment->screen());

            if (desktop && desktop->immutability() == Plasma::Mutable) {
                actionList << ActionList::createActionItem(i18n("Add to Desktop"), "addToDesktop");
            }

            if (containment->immutability() == Plasma::Mutable) {
                actionList << ActionList::createActionItem(i18n("Add to Panel"), "addToPanel");
            }

            QObject* taskManager = 0;

            foreach(QObject* applet, containment->applets()) {
                if (applet->metaObject()->indexOfSlot("hasLauncher(QString)") != -1) {
                    taskManager = applet;
                }
            }

            if (taskManager) {
                bool hasLauncher = false;

                QMetaObject::invokeMethod(taskManager, "hasLauncher", Qt::DirectConnection,
                    Q_RETURN_ARG(bool, hasLauncher), Q_ARG(QString, storageId));

                if (!hasLauncher) {
                    actionList << ActionList::createActionItem(i18n("Add as Launcher"), "addLauncher");
                }
            }
        }

        return actionList;
    }

    return QVariant();
}

void RecentAppsModel::addApp(const QString& storageId, bool sync)
{
    int index = m_storageIdList.indexOf(storageId);

    if (index > 0) {
        beginMoveRows(QModelIndex(), index, index, QModelIndex(), 0);
        m_storageIdList.move(index, 0);
        endMoveRows();
    } else if (index == -1) {
        if (m_storageIdList.count() < 15) {
            beginInsertRows(QModelIndex(), 0, 0);
            m_storageIdList.prepend(storageId);
            endInsertRows();
            emit countChanged();
        } else {
            beginResetModel();
            m_storageIdList.prepend(storageId);
            m_storageIdList.removeLast();
            endResetModel();
        }
    }

    if (sync) {
        m_configGroup.writeEntry("RecentApps", m_storageIdList);
        m_configGroup.config()->sync();
    }
}

bool RecentAppsModel::forgetApp(int row, bool sync)
{
    if (row >= m_storageIdList.count()) {
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row);
    m_storageIdList.removeAt(row);
    endRemoveRows();

    emit countChanged();

    if (sync) {
        m_configGroup.writeEntry("RecentApps", m_storageIdList);
        m_configGroup.config()->sync();
    }

    return false;
}

bool RecentAppsModel::trigger(int row, const QString &actionId, const QVariant &actionArgument)
{
    Q_UNUSED(actionArgument)

    if (row >= m_storageIdList.count()) {
        return false;
    }

    const QString storageId = m_storageIdList.at(row);

    if (!actionId.isEmpty()) {
        if (actionId == "forget") {
            return forgetApp(row);
        } else {
            QVariant adaptor = qApp->property("HomerunViewerAdaptor");

            if (adaptor.isValid()) {
                uint containmentId = (actionId == "addToDesktop") ? qApp->property("desktopContainmentId").toUInt()
                    : qApp->property("appletContainmentId").toUInt();
                return QMetaObject::invokeMethod(adaptor.value<QObject *>(), actionId.toLocal8Bit(),
                    Qt::DirectConnection, Q_ARG(uint, containmentId), Q_ARG(QString, storageId));
            } else if (m_containment) {
                Plasma::Containment *containment = static_cast<Plasma::Containment *>(m_containment);
                KService::Ptr service = KService::serviceByStorageId(storageId);

                if (actionId == "addToDesktop" && service) {
                    Plasma::Containment *desktop = containment->corona()->containmentForScreen(containment->screen());

                    if (desktop) {
                        if (desktop->metaObject()->indexOfSlot("addUrls(KUrl::List)") != -1) {
                            QMetaObject::invokeMethod(desktop, "addUrls",
                            Qt::DirectConnection, Q_ARG(KUrl::List, KUrl::List(service->entryPath())));
                        } else {
                            desktop->addApplet("icon", QVariantList() << service->entryPath());
                        }
                    }
                } else if (actionId == "addToPanel" && service) {
                    QRectF rect(containment->geometry().width() / 2, 0, 150,
                        containment->boundingRect().height());
                    containment->addApplet("icon", QVariantList() << service->entryPath(), rect);
                } else if (actionId == "addLauncher") {
                    QObject* taskManager = 0;

                    foreach(QObject* applet, containment->applets()) {
                        if (applet->metaObject()->indexOfSlot("addLauncher(QString)") != -1) {
                            taskManager = applet;
                        }
                    }

                    if (taskManager) {
                        QMetaObject::invokeMethod(taskManager, "addLauncher", Qt::DirectConnection,
                            Q_ARG(QString, storageId));
                    }
                }
            }
        }
    } else {
        KService::Ptr service = KService::serviceByStorageId(storageId);

        if (!service) {
            return false;
        }

        bool ran = KRun::run(*service, KUrl::List(), 0);

        if (ran) {
            addApp(storageId);
        }

        return ran;
    }

    return false;
}

QString RecentAppsModel::name() const
{
    return i18n("Recent Applications");
}

//- RecentAppsSource ---------------------------------------------
RecentAppsSource::RecentAppsSource(QObject *parent)
: AbstractSource(parent)
{}

QAbstractItemModel *RecentAppsSource::createModelFromConfigGroup(const KConfigGroup &group)
{
    return new RecentAppsModel(group);
}

} // namespace Homerun

#include "recentappsmodel.moc"
