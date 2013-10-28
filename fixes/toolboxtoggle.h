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

#ifndef TOOLBOXTOGGLE_H
#define TOOLBOXTOGGLE_H

#include <QObject>

class QGraphicsWidget;

class ToolBoxToggle : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* toolBox READ toolBox WRITE setToolBox)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)

    public:
        ToolBoxToggle(QObject *parent = 0);
        ~ToolBoxToggle();

        QObject *toolBox() const;
        void setToolBox(QObject* toolBox);

        bool visible() const;
        void setVisible(bool visible);

    Q_SIGNALS:
        void toolBoxChanged();
        void visibleChanged();

    private:
        QGraphicsWidget *m_toolBox;
        bool m_visible;
};

#endif
