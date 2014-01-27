/*
 *   Copyright 2013 Eike Hein <hein@kde.org>
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "appletproxy.h"

#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/PopupApplet>

AppletProxy::AppletProxy(QObject* parent) : QObject(parent),
    m_applet(0)
{
    KGlobal::locale()->insertCatalog("plasma_applet_org.kde.homerun");
}

AppletProxy::~AppletProxy()
{
}

QObject* AppletProxy::item() const
{
    return m_applet;
}

void AppletProxy::setItem(QObject *item)
{
    if (!item || !item->parent() || !item->parent()->parent()) {
        return;
    }

    m_applet = static_cast<Plasma::PopupApplet *>(item->parent()->parent());
}

QObject *AppletProxy::containment() const
{
    if (m_applet) {
        return m_applet->containment();
    }

    return 0;
}

#include "appletproxy.moc"
