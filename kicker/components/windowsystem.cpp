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
#include <QRect>

#include <KWindowSystem>

#include <Plasma/Plasma>

Margins::Margins(QObject* parent) : QObject(parent)
, m_left(0)
, m_top(0)
, m_right(0)
, m_bottom(0)
{
}

Margins::~Margins()
{
}

int Margins::left() const
{
    return m_left;
}

void Margins::setLeft(int left)
{
    if (left != m_left) {
        m_left = left;
        emit leftChanged();
    }
}

int Margins::top() const
{
    return m_top;
}

void Margins::setTop(int top)
{
    if (top != m_top) {
        m_top = top;
        emit topChanged();
    }
}

int Margins::right() const
{
    return m_right;
}

void Margins::setRight(int right)
{
    if (right != m_right) {
        m_right = right;
        emit rightChanged();
    }
}

int Margins::bottom() const
{
    return m_bottom;
}

void Margins::setBottom(int bottom)
{
    if (bottom != m_bottom) {
        m_bottom = bottom;
        emit bottomChanged();
    }
}

WindowSystem::WindowSystem(QObject* parent) : QObject(parent)
, m_margins(new Margins(this))
{
}

WindowSystem::~WindowSystem()
{
}

QObject* WindowSystem::margins() const
{
    return m_margins;
}

void WindowSystem::updateMargins(QDeclarativeItem *item)
{
    QGraphicsView *view = Plasma::viewFor(item);

    if (!view || !view->parentWidget()) {
        return;
    }

    int left, top, right, bottom;

    view->parentWidget()->getContentsMargins(&left, &top, &right, &bottom);

    m_margins->setLeft(left);
    m_margins->setTop(top);
    m_margins->setRight(right);
    m_margins->setBottom(bottom);
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

#include "windowsystem.moc"
