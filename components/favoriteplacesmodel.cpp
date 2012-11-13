/*
Copyright 2012 Aurélien Gâteau <agateau@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "favoriteplacesmodel.h"

// Local
#include <dirmodel.h>

// KDE
#include <KDebug>
#include <KLocale>

// Qt

namespace Homerun {

KUrl FavoritePlacesModel::urlFromFavoriteId(const QString &favoriteId)
{
    if (!favoriteId.startsWith("place:")) {
        kWarning() << "Wrong favoriteId" << favoriteId;
        return QString();
    }
    return KUrl(favoriteId.mid(6));
}

QString FavoritePlacesModel::favoriteIdFromUrl(const KUrl &url)
{
    return "place:" + url.url();
}

//- FavoritePlacesModel ------------------------------------------------
FavoritePlacesModel::FavoritePlacesModel(QObject *parent)
: KFilePlacesModel(parent)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::DecorationRole, "decoration");
    roles.insert(FavoritePlacesModel::FavoriteIdRole, "favoriteId");
    setRoleNames(roles);
}

QString FavoritePlacesModel::favoritePrefix() const
{
    return "place";
}

bool FavoritePlacesModel::isFavorite(const QString &favoriteId) const
{
    return indexForFavoriteId(favoriteId).isValid();
}

void FavoritePlacesModel::addFavorite(const QString &favoriteId)
{
    KUrl favoriteUrl = urlFromFavoriteId(favoriteId);
    if (favoriteUrl.isEmpty()) {
        return;
    }
    addPlace(favoriteUrl.fileName(), favoriteUrl);
}

void FavoritePlacesModel::removeFavorite(const QString &favoriteId)
{
    QModelIndex index = indexForFavoriteId(favoriteId);
    if (!index.isValid()) {
        kWarning() << "No favorite place for" << favoriteId;
        return;
    }
    removePlace(index);
}

QModelIndex FavoritePlacesModel::indexForFavoriteId(const QString &favoriteId) const
{
    KUrl favoriteUrl = urlFromFavoriteId(favoriteId);
    if (favoriteUrl.isEmpty()) {
        return QModelIndex();
    }
    for (int row = rowCount() - 1; row >= 0; --row) {
        QModelIndex idx = index(row, 0);
        if (url(idx).equals(favoriteUrl, KUrl::CompareWithoutTrailingSlash)) {
            return idx;
        }
    }
    return QModelIndex();
}

QVariant FavoritePlacesModel::data(const QModelIndex &index, int role) const
{
    if (role != FavoriteIdRole) {
        return KFilePlacesModel::data(index, role);
    }

    if (index.row() < 0 || index.row() >= rowCount()) {
        return QVariant();
    }
    return QVariant(favoriteIdFromUrl(url(index)));
}

bool FavoritePlacesModel::trigger(int row)
{
    QModelIndex idx = index(row, 0);

    KUrl theUrl = idx.data(KFilePlacesModel::UrlRole).value<QUrl>();
    theUrl.adjustPath(KUrl::AddTrailingSlash);
    QString rootName = idx.data(Qt::DisplayRole).toString();
    openSourceRequested("Dir", DirModel::sourceArguments(theUrl, rootName, theUrl));

    return false;
}

QString FavoritePlacesModel::name() const
{
    return i18n("Favorite Places");
}

int FavoritePlacesModel::count() const
{
    return rowCount(QModelIndex());
}

} // namespace Homerun

#include "favoriteplacesmodel.moc"
