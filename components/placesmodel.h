/*
 * Copyright 2012 Aurélien Gâteau <agateau@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef PLACESMODEL_H
#define PLACESMODEL_H

#include <KFilePlacesModel>

/**
 * Adapts KFilePlacesModel to make it SAL friendly
 */
class PlacesModel : public KFilePlacesModel
{
    Q_OBJECT

public:
    PlacesModel(QObject *parent = 0);
    Q_INVOKABLE void run(int row);
};

#endif /* PLACESMODEL_H */
