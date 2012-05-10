/*
 * Copyright (c) 2007      Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
 * Copyright (c) 2002,2003 Hamish Rodda <rodda@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __RANDRDISPLAY_H__
#define __RANDRDISPLAY_H__

#include "randr.h"

//#include <X11/Xlib.h>

/**
 * Just wraps the X Display and provides access to RandRScreens
 */
class RandRDisplay
{
public:
    RandRDisplay();
    ~RandRDisplay();

    bool isValid() const;
    const QString& errorCode() const;
    const QString& version() const;

    /**
     * Event and error code base offsets
     */
    int eventBase() const;
    int errorBase() const;

    int screenIndexOfWidget(QWidget* widget);

    int numScreens() const;
    RandRScreen* screen(int index);
    RandRScreen* currentScreen();

    void setCurrentScreen(int index);
    int    currentScreenIndex() const;

    bool needsRefresh() const;
    void refresh();

    void applyProposed(bool confirm = true);

    bool canHandle(const XEvent *e) const;
    void handleEvent(XEvent *e);

private:
    Display *m_dpy;
    int    m_numScreens;
    int    m_currentScreenIndex;

    ScreenList m_screens;

    bool m_valid;
    QString    m_errorCode;
    QString    m_version;

    int    m_eventBase;
    int m_errorBase;
};

#endif
