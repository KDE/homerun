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
#ifndef HELPMENUACTIONS_H
#define HELPMENUACTIONS_H

// Local

// Qt

// KDE
#include <KHelpMenu>

/**
 * Wrapper exposing KHelpMenu actions
 */
class HelpMenuActions : public QObject
{
    Q_OBJECT
    Q_ENUMS(ActionId)
public:
    explicit HelpMenuActions(QObject *parent = 0);
    ~HelpMenuActions();

    enum ActionId {
        AboutApplication = KHelpMenu::menuAboutApp,
        AboutKDE = KHelpMenu::menuAboutKDE,
        ReportBug = KHelpMenu::menuReportBug,
        SwitchLanguage = KHelpMenu::menuSwitchLanguage,
    };

    Q_INVOKABLE QString text(ActionId);
    Q_INVOKABLE void trigger(ActionId);

private:
    KHelpMenu *m_menu;
    KAboutData *m_aboutData;

    QAction *action(ActionId);
};

#endif /* HELPMENUACTIONS_H */
