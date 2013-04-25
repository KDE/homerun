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

namespace Homerun {

enum Action
{
    SuspendToRam,
    SuspendToDisk,
    Reboot,
    Halt
};

PowerModel::PowerModel(QObject *parent)
: StandardItemModel(parent)
{
    setName(i18n("Power"));
    QSet<Solid::PowerManagement::SleepState> sleepStates = Solid::PowerManagement::supportedSleepStates();

    if (sleepStates.contains(Solid::PowerManagement::SuspendState)) {
        StandardItem *item = new StandardItem(i18nc("an action", "Suspend"), "system-suspend");
        item->setData(SuspendToRam);
        appendRow(item);
    }

    if (sleepStates.contains(Solid::PowerManagement::HibernateState)) {
        StandardItem *item = new StandardItem(i18nc("an action", "Hibernate"), "system-suspend-hibernate");
        item->setData(SuspendToDisk);
        appendRow(item);
    }

    {
        StandardItem *item = new StandardItem(i18nc("an action", "Restart"), "system-reboot");
        item->setData(Reboot);
        appendRow(item);
    }
    {
        StandardItem *item = new StandardItem(i18nc("an action", "Shutdown"), "system-shutdown");
        item->setData(Halt);
        appendRow(item);
    }
}

bool PowerModel::trigger(int row, const QString &/*actionId*/, const QVariant &/*actionArgument*/)
{
    QStandardItem *itm = item(row);
    Q_ASSERT(itm);
    Action action = static_cast<Action>(itm->data().toInt());
    switch (action) {
    case SuspendToRam:
         Solid::PowerManagement::requestSleep(Solid::PowerManagement::SuspendState, 0, 0);
        break;
    case SuspendToDisk:
         Solid::PowerManagement::requestSleep(Solid::PowerManagement::HibernateState, 0, 0);
        break;
    case Halt:
        KWorkSpace::requestShutDown(KWorkSpace::ShutdownConfirmDefault, KWorkSpace::ShutdownTypeHalt);
        break;
    case Reboot:
        KWorkSpace::requestShutDown(KWorkSpace::ShutdownConfirmDefault, KWorkSpace::ShutdownTypeReboot);
        break;
    }
    return true;
}

} // namespace Homerun

#include "powermodel.moc"
