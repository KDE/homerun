/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 André Duffeck <duffeck@kde.org>
 *   Copyright 2008 Chani Armitage <chanika@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SALVIEW_H
#define SALVIEW_H

#include <QWeakPointer>

#include <Plasma/Plasma>
#include <Plasma/View>

namespace Plasma
{
    class Containment;
    class WidgetExplorer;
}

class SalView : public Plasma::View
{
    Q_OBJECT

public:
    SalView(Plasma::Containment* containment, QWidget *parent);
    ~SalView();

protected:
    void drawBackground(QPainter *painter, const QRectF & rect);
};

#endif // multiple inclusion guard
