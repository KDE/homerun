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

bool AppletProxy::desktopContainmentMutable() const
{
    if (m_applet && m_applet->containment()) {
        Plasma::Containment *desktop = m_applet->containment()->corona()->containmentForScreen(m_applet->containment()->screen());

        if (desktop) {
            return desktop->immutability() == Plasma::Mutable;
        }
    }

    return false;
}

bool AppletProxy::appletContainmentMutable() const
{
    if (m_applet && m_applet->containment()) {
        return m_applet->containment()->immutability() == Plasma::Mutable;
    }

    return false;
}

void AppletProxy::addToDesktop(const QString &storageId)
{
    if (!m_applet) {
        return;
    }

    Plasma::Containment *desktop = m_applet->containment()->corona()->containmentForScreen(m_applet->containment()->screen());
    KService::Ptr service = KService::serviceByStorageId(storageId);

    if (!desktop || !service) {
        return;
    }

    if (desktop->metaObject()->indexOfSlot("addUrls(KUrl::List)") != -1) {
        QMetaObject::invokeMethod(desktop, "addUrls",
        Qt::DirectConnection, Q_ARG(KUrl::List, KUrl::List(service->entryPath())));
    } else {
        desktop->addApplet("icon", QVariantList() << service->entryPath());
    }
}

void AppletProxy::addToPanel(const QString &storageId)
{
    if (!m_applet) {
        return;
    }

    KService::Ptr service = KService::serviceByStorageId(storageId);

    if (service) {
        // move it to the middle of the panel
        QRectF rect(m_applet->containment()->geometry().width() / 2, 0, 150,
            m_applet->containment()->boundingRect().height());
        m_applet->containment()->addApplet("icon", QVariantList() << service->entryPath(), rect);
    }
}

#include "appletproxy.moc"
