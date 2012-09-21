/*
    Copyright 2012 Aurélien Gâteau <agateau@kde.org>
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

// Own
#include "powermodel.h"

//Qt
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusPendingCall>

// KDE
#include <KDebug>
#include <KIcon>
#include <KLocale>
#include <KRun>
#include <KService>
#include <kworkspace/kworkspace.h>

//Plasma
#include <Plasma/AbstractRunner>
#include <Plasma/RunnerManager>

namespace Homerun {

PowerModel::PowerModel(QObject *parent)
: QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::DecorationRole, "icon");
    setRoleNames(roles);

    PowerAction shutdown;
    shutdown.name = i18nc("an action", "Shutdown");
    shutdown.type = Shutdown;
    shutdown.iconName = "system-shutdown";
    m_powerList.append(shutdown);

    PowerAction restart;
    restart.name = i18nc("an action", "Restart");
    restart.type = Restart;
    restart.iconName = "system-reboot";
    m_powerList.append(restart);

    PowerAction suspend;
    suspend.name = i18nc("an action", "Suspend");
    suspend.type = Suspend;
    suspend.iconName = "system-suspend";
    m_powerList.append(suspend);

    PowerAction hibernate;
    hibernate.name = i18nc("an action", "Hibernate");
    hibernate.type = Hibernate;
    hibernate.iconName = "system-suspend-hibernate";
    m_powerList.append(hibernate);
    emit countChanged();
}

PowerModel::~PowerModel()
{
}

int PowerModel::count() const
{
    return m_powerList.count();
}

QString PowerModel::name() const
{
    return i18n("Power");
}

int PowerModel::rowCount(const QModelIndex &index) const
{
    if (index.isValid()) {
        return 0;
    }
    return m_powerList.count();
}

QVariant PowerModel::data(const QModelIndex &index, int role) const
{
    PowerAction action = m_powerList.value(index.row());

    if (role == Qt::DisplayRole) {
        return action.name;
    } else if (role == Qt::DecorationRole) {
        return KIcon(action.iconName);
    } else {
        kWarning() << "Unhandled role" << role;
        return QVariant();
    }
}

bool PowerModel::trigger(int row)
{
    PowerAction action = m_powerList.value(row);

    switch (action.type) {
        case Shutdown:
            KWorkSpace::requestShutDown();
            break;

        case Restart:
            KWorkSpace::requestShutDown(KWorkSpace::ShutdownConfirmDefault, KWorkSpace::ShutdownTypeReboot, KWorkSpace::ShutdownModeDefault);
            break;

        case Suspend:
            suspend("suspendToRam");
            break;

        case Hibernate:
            suspend("suspendToDisk");
            break;
    }

    return true;
}

void PowerModel::suspend(const QString& type)
{
    QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.Solid.PowerManagement",
                                                                                                    "/org/kde/Solid/PowerManagement",
                                                                                                    "org.kde.Solid.PowerManagement",
                                                                                                    type);
    QDBusConnection::sessionBus().asyncCall(msg);
}

} // namespace Homerun

#include "powermodel.moc"
