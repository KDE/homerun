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

AbstractSessionAction::~AbstractSessionAction()
{
}

class LockSessionAction : public AbstractSessionAction
{
public:
    LockSessionAction()
    {
        name = i18nc("an action", "Lock");
        iconName = "system-lock-screen";
    }

    void run() override
    {
        lockSession();
    }
};

class NewSessionAction : public AbstractSessionAction
{
public:
    NewSessionAction(KDisplayManager *manager)
    : m_displayManager(manager)
    {
        name = i18nc("an action", "New Session");
        iconName = "system-switch-user";
    }

    void run() override
    {
        lockSession();
        m_displayManager->startReserve();
    }

private:
    KDisplayManager *m_displayManager;
};

class LogoutAction : public AbstractSessionAction
{
public:
    LogoutAction()
    {
        name = i18nc("an action", "Logout");
        iconName = "system-log-out";
    }

    void run() override
    {
        KWorkSpace::requestShutDown(KWorkSpace::ShutdownConfirmDefault, KWorkSpace::ShutdownTypeNone);
    }
};

SessionModel::SessionModel(QObject *parent)
: QAbstractListModel(parent)
{
    //FIXME: instead of just hiding these things..it'd be awesome if we could grey them out and/or provide a reason why they're not there.
    //otherwise the user is hunting for the power buttons and for some reason it isn't where it should be.
    if (KAuthorized::authorizeKAction("lock_screen")) {
        m_sessionList.append(new LockSessionAction);
    }

    if (KAuthorized::authorizeKAction("start_new_session")
        && m_displayManager.isSwitchable()
        && m_displayManager.numReserve() >= 0)
    {
        m_sessionList.append(new NewSessionAction(&m_displayManager));
    }

    if (KAuthorized::authorizeKAction("logout") && KAuthorized::authorize("logout")) {
        m_sessionList.append(new LogoutAction);
    }
}

SessionModel::~SessionModel()
{
    qDeleteAll(m_sessionList);
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
    AbstractSessionAction *action = m_sessionList.value(index.row());
    if (!action) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return action->name;
    } else if (role == Qt::DecorationRole) {
        return action->iconName;
    } else {
        kWarning() << "Unhandled role" << role;
        return QVariant();
    }
}

bool SessionModel::trigger(int row)
{
    AbstractSessionAction *action = m_sessionList.value(row);
    Q_ASSERT(action);
    action->run();
    return true;
}

} // namespace Homerun

#include "sessionmodel.moc"
