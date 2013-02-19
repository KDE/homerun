/*
    Copyright 2012 Aurélien Gâteau <agateau@kde.org>
    Copyright 2012 (C) Shaun Reich <shaun.reich@blue-systems.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef POWERMODEL_H
#define POWERMODEL_H

// Local
#include <standarditemmodel.h>

namespace Homerun {

class PowerModel : public StandardItemModel
{
    Q_OBJECT

public:
    PowerModel(QObject *parent = 0);
    bool trigger(int row, const QString &/*actionId*/, const QVariant &/*actionArgument*/) override;
};

} // namespace Homerun

#endif
