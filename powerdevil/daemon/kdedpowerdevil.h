/***************************************************************************
 *   Copyright (C) 2010 by Dario Freddi <drf@kde.org>                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef KDEDPOWERDEVIL_H
#define KDEDPOWERDEVIL_H

#include <KDEDModule>
#include <QVariantList>

namespace PowerDevil {
    class Core;
}

class KDEDPowerDevil : public KDEDModule
{
    Q_OBJECT
    Q_DISABLE_COPY(KDEDPowerDevil)

public:
    explicit KDEDPowerDevil(QObject* parent, const QVariantList &);
    virtual ~KDEDPowerDevil();

private Q_SLOTS:
    void init();
    void onCoreReady();

private:
    PowerDevil::Core *m_core;
};

#endif // KDEDPOWERDEVIL_H
