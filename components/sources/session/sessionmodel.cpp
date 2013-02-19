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
#include "sessionmodel.h"

//Qt
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCall>

// KDE
#include <KAuthorized>
#include <KDebug>
#include <KIcon>
#include <KLocale>

namespace Homerun {

static void lockSession()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    QDBusInterface interface("org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver", bus);
    interface.asyncCall("Lock");
}

SessionModel::SessionModel(QObject *parent)
: QAbstractListModel(parent)
{
    //FIXME: instead of just hiding these things..it'd be awesome if we could grey them out and/or provide a reason why they're not there.
    //otherwise the user is hunting for the power buttons and for some reason it isn't where it should be.
    if (KAuthorized::authorizeKAction("lock_screen")) {
        SessionAction lock;
        lock.name = i18nc("an action", "Lock");
        lock.type = Lock;
        lock.iconName = "system-lock-screen";
        m_sessionList.append(lock);
    }

    if (KAuthorized::authorizeKAction("start_new_session")
        && m_displayManager.isSwitchable()
        && m_displayManager.numReserve() >= 0)
    {
        SessionAction action;
        action.name = i18nc("an action", "New Session");
        action.type = StartNewSession;
        action.iconName = "system-switch-user";
        m_sessionList.append(action);
    }

    const bool canLogout = KAuthorized::authorizeKAction("logout") && KAuthorized::authorize("logout");
    if (canLogout) {
        SessionAction logout;
        logout.name = i18nc("an action", "Logout");
        logout.type = Logout;
        logout.iconName = "system-log-out";
        m_sessionList.append(logout);
    }
}

SessionModel::~SessionModel()
{
}

int SessionModel::count() const
{
    return m_sessionList.count();
}

QString SessionModel::name() const
{
    return i18n("Session");
}

int SessionModel::rowCount(const QModelIndex &index) const
{
    if (index.isValid()) {
        return 0;
    }
    return m_sessionList.count();
}

QVariant SessionModel::data(const QModelIndex &index, int role) const
{
    SessionAction action = m_sessionList.value(index.row());

    if (role == Qt::DisplayRole) {
        return action.name;
    } else if (role == Qt::DecorationRole) {
        return KIcon(action.iconName);
    } else {
        kWarning() << "Unhandled role" << role;
        return QVariant();
    }
}

bool SessionModel::trigger(int row)
{
    SessionAction action = m_sessionList.value(row);

    switch (action.type) {
    case Logout:
        KWorkSpace::requestShutDown(KWorkSpace::ShutdownConfirmDefault, KWorkSpace::ShutdownTypeNone);
        break;

    case StartNewSession:
        startNewSession();
        break;

    case Lock:
        lockSession();
        break;
    }

    return true;
}

void SessionModel::startNewSession()
{
    lockSession();
    m_displayManager.startReserve();
}

} // namespace Homerun

#include "sessionmodel.moc"
