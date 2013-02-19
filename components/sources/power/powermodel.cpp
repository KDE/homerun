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

// KDE
#include <KDebug>
#include <KLocale>
#include <kworkspace/kworkspace.h>
#include <Solid/PowerManagement>

// Qt
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusPendingCall>

namespace Homerun {

class SuspendItem : public StandardItem
{
public:
    SuspendItem(const QString &text, const QString &iconName, const QString &type)
    : StandardItem(text, iconName)
    , m_type(type)
    {}

    bool trigger(const QString &/*actionId*/, const QVariant &/*actionArgument*/) override
    {
        QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.Solid.PowerManagement",
                                                        "/org/kde/Solid/PowerManagement",
                                                        "org.kde.Solid.PowerManagement",
                                                        m_type);
        QDBusConnection::sessionBus().asyncCall(msg);
        return true;
    }
private:
    QString m_type;
};

class ShutdownItem : public StandardItem
{
public:
    ShutdownItem(const QString &text, const QString &iconName, const KWorkSpace::ShutdownType &type)
    : StandardItem(text, iconName)
    , m_type(type)
    {}

    bool trigger(const QString &/*actionId*/, const QVariant &/*actionArgument*/) override
    {
        KWorkSpace::requestShutDown(KWorkSpace::ShutdownConfirmDefault, m_type);
        return true;
    }
private:
    KWorkSpace::ShutdownType m_type;
};

PowerModel::PowerModel(QObject *parent)
: StandardItemModel(parent)
{
    setName(i18n("Power"));
    QSet<Solid::PowerManagement::SleepState> sleepStates = Solid::PowerManagement::supportedSleepStates();

    if (sleepStates.contains(Solid::PowerManagement::SuspendState)) {
        appendRow(new SuspendItem(
            i18nc("an action", "Suspend"),
            "system-suspend",
            "suspendToRam")
        );
    }

    if (sleepStates.contains(Solid::PowerManagement::HibernateState)) {
        appendRow(new SuspendItem(
            i18nc("an action", "Hibernate"),
            "system-suspend-hibernate",
            "suspendToDisk")
        );
    }

    appendRow(new ShutdownItem(
        i18nc("an action", "Restart"),
        "system-reboot",
        KWorkSpace::ShutdownTypeReboot)
    );

    appendRow(new ShutdownItem(
        i18nc("an action", "Shutdown"),
        "system-shutdown",
        KWorkSpace::ShutdownTypeHalt)
    );
}

} // namespace Homerun

#include "powermodel.moc"
