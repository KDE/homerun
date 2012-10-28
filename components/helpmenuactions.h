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
        ReportBug = KHelpMenu::menuReportBug,
    };

    Q_INVOKABLE QString text(ActionId);
    Q_INVOKABLE void trigger(ActionId);

private:
    KHelpMenu *m_menu;

    QAction *action(ActionId);
};

#endif /* HELPMENUACTIONS_H */