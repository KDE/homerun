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

#ifndef WINDOWSYSTEM_H
#define WINDOWSYSTEM_H

#include <QObject>
#include <QPoint>

class QDeclarativeItem;

class Margins : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int left READ left NOTIFY leftChanged)
    Q_PROPERTY(int top READ top NOTIFY topChanged)
    Q_PROPERTY(int right READ right NOTIFY rightChanged)
    Q_PROPERTY(int bottom READ bottom NOTIFY bottomChanged)

    public:
        Margins(QObject *parent);
        ~Margins();

        int left() const;
        void setLeft(int left);

        int top() const;
        void setTop(int top);

        int right() const;
        void setRight(int right);

        int bottom() const;
        void setBottom(int bottom);

    signals:
        void leftChanged();
        void topChanged();
        void rightChanged();
        void bottomChanged();

    private:
        int m_left;
        int m_top;
        int m_right;
        int m_bottom;
};

class WindowSystem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* margins READ margins CONSTANT)

    public:
        WindowSystem(QObject *parent = 0);
        ~WindowSystem();

        QObject *margins() const;

        Q_INVOKABLE void updateMargins(QDeclarativeItem *item);

        Q_INVOKABLE QVariant workArea();

        Q_INVOKABLE QPoint mapToScreen(QDeclarativeItem *item, int x, int y);

    private:

        Margins *m_margins;
};

#endif
