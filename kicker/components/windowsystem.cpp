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

#include "windowsystem.h"

#include <QDeclarativeItem>
#include <QGraphicsView>

#include <KWindowSystem>

#include <Plasma/Plasma>

WindowSystem::WindowSystem(QObject* parent) : QObject(parent)
{
}

WindowSystem::~WindowSystem()
{
}

QVariant WindowSystem::workArea()
{
    return KWindowSystem::workArea();
}

QPoint WindowSystem::mapToScreen(QDeclarativeItem *item, int x, int y)
{
    QPoint pos(x, y);

    if (!item) {
        return pos;
    }

    QGraphicsView *view = Plasma::viewFor(item);

    return view->mapToGlobal(view->mapFromScene(item->mapToScene(pos).toPoint()));
}

void WindowSystem::raiseParentWindow(QDeclarativeItem *item)
{
    QGraphicsView *view = Plasma::viewFor(item);

    if (!view || !view->window()) {
        return;
    }

    KWindowSystem::forceActiveWindow(Plasma::viewFor(static_cast<QDeclarativeItem *>(parent()))->window()->winId());
    KWindowSystem::raiseWindow(view->window()->winId());
}

#include "windowsystem.moc"
