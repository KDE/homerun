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

#ifndef APPLETPROXY_H
#define APPLETPROXY_H

#include <QObject>

namespace Plasma {
    class PopupApplet;
}

class AppletProxy : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* item READ item WRITE setItem)
    Q_PROPERTY(QObject* containment READ containment)

    public:
        AppletProxy(QObject *parent = 0);
        ~AppletProxy();

        QObject *item() const;
        void setItem(QObject* item);

        QObject *containment() const;

    private:
        Plasma::PopupApplet *m_applet;
};

#endif
