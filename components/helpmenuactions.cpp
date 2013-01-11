/*
Copyright 2012 Aurélien Gâteau <agateau@kde.org>

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
// Self
#include <helpmenuactions.h>

// Local

// KDE
#include <KAboutData>

// Qt
#include <QAction>
#include <QApplication>

// Local
#include <aboutdata.h>

HelpMenuActions::HelpMenuActions(QObject *parent)
: QObject(parent)
, m_menu(0)
, m_aboutData(0)
{
}

HelpMenuActions::~HelpMenuActions()
{
    delete m_menu;
    delete m_aboutData;
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
        m_aboutData = HomerunInternal::createAboutData();
        m_menu = new KHelpMenu(QApplication::activeWindow(), m_aboutData);
        // Call menu() to cause the actions to be created
        m_menu->menu();
    }
    return m_menu->action(static_cast<KHelpMenu::MenuId>(actionId));
}

#include <helpmenuactions.moc>
