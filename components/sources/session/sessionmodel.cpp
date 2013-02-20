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

// Local

// KDE
#include <KAuthorized>
#include <KDebug>
#include <KLocale>

// Qt
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCall>

namespace Homerun {

static void lockSession()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    QDBusInterface interface("org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver", bus);
    interface.asyncCall("Lock");
}

//- Local items -------------------------------------------------------------------
class LockSessionItem : public StandardItem
{
public:
    LockSessionItem()
    : StandardItem(i18nc("an action", "Lock"), "system-lock-screen")
    {
    }

    bool trigger(const QString &/*actionId*/, const QVariant &/*actionArgument*/) override
    {
        lockSession();
        return true;
    }
};

class NewSessionItem : public StandardItem
{
public:
    NewSessionItem(KDisplayManager *manager)
    : StandardItem(i18nc("an action", "New Session"), "system-switch-user")
    , m_displayManager(manager)
    {
    }

    bool trigger(const QString &/*actionId*/, const QVariant &/*actionArgument*/) override
    {
        lockSession();
        m_displayManager->startReserve();
        return true;
    }

private:
    KDisplayManager *m_displayManager;
};

class LogoutItem : public StandardItem
{
public:
    LogoutItem()
    : StandardItem(i18nc("an action", "Logout"), "system-log-out")
    {}

    bool trigger(const QString &/*actionId*/, const QVariant &/*actionArgument*/) override
    {
        KWorkSpace::requestShutDown(KWorkSpace::ShutdownConfirmDefault, KWorkSpace::ShutdownTypeNone);
        return true;
    }
};

SessionModel::SessionModel(QObject *parent)
: StandardItemModel(parent)
{
    setName(i18n("Session"));

    if (KAuthorized::authorizeKAction("lock_screen")) {
        appendRow(new LockSessionItem);
    }

    if (KAuthorized::authorizeKAction("logout") && KAuthorized::authorize("logout")) {
        appendRow(new LogoutItem);
    }

    if (KAuthorized::authorizeKAction("start_new_session")
        && m_displayManager.isSwitchable()
        && m_displayManager.numReserve() >= 0)
    {
        appendRow(new NewSessionItem(&m_displayManager));
    }
}

} // namespace Homerun

#include "sessionmodel.moc"
