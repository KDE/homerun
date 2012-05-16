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
#include "placesmodel.h"

#include <KDebug>
#include <KMimeType>
#include <KRun>

PlacesModel::PlacesModel(QObject *parent)
: KFilePlacesModel(parent)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "label");
    roles.insert(Qt::DecorationRole, "icon");
    roles.insert(KFilePlacesModel::UrlRole, "url");
    setRoleNames(roles);
}

void PlacesModel::run(int row)
{
    KUrl theUrl = url(index(row, 0));
    KMimeType::Ptr mimeTypePtr = KMimeType::findByUrl(theUrl);
    if (!mimeTypePtr) {
        kError() << "Unable to determine mimetype for" << theUrl;
        return;
    }
    KRun::runUrl(theUrl, mimeTypePtr->name(), 0);
}

#include "placesmodel.moc"
