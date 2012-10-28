/*
 * Copyright 2012 Aurélien Gâteau <agateau@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
// Self
#include <helpmenuactions.h>

// Local

// KDE
#include <KAboutData>
#include <KComponentData>
#include <KGlobal>

// Qt
#include <QAction>
#include <QApplication>

HelpMenuActions::HelpMenuActions(QObject *parent)
: QObject(parent)
, m_menu(0)
{
}

HelpMenuActions::~HelpMenuActions()
{
    delete m_menu;
}

QString HelpMenuActions::text(HelpMenuActions::ActionId actionId)
{
    QAction *act = action(actionId);
    Q_ASSERT(act);
    return act->text();
}

void HelpMenuActions::trigger(HelpMenuActions::ActionId actionId)
{
    QAction *act = action(actionId);
    Q_ASSERT(act);
    act->trigger();
}

QAction *HelpMenuActions::action(HelpMenuActions::ActionId actionId)
{
    if (!m_menu) {
        m_menu = new KHelpMenu(QApplication::activeWindow(), KGlobal::mainComponent().aboutData());
        // Call menu() to cause the actions to be created
        m_menu->menu();
    }
    return m_menu->action(static_cast<KHelpMenu::MenuId>(actionId));
}

#include <helpmenuactions.moc>
