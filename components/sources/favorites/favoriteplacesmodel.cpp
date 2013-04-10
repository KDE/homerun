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
#include <favoriteutils.h>

// libhomerun
#include <actionlist.h>

// KDE
#include <KDebug>
#include <KFileItem>
#include <KLocale>

// Qt

namespace Homerun {

//- FavoritePlacesModel ------------------------------------------------
FavoritePlacesModel::FavoritePlacesModel(QObject *parent)
: Fixes::KFilePlacesModel(parent)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::DecorationRole, "decoration");
    roles.insert(FavoriteIdRole, "favoriteId");
    roles.insert(HasActionListRole, "hasActionList");
    roles.insert(ActionListRole, "actionList");
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
    KUrl favoriteUrl = FavoriteUtils::urlFromFavoriteId(favoriteId);
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
    KUrl favoriteUrl = FavoriteUtils::urlFromFavoriteId(favoriteId);
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
    if (role != FavoriteIdRole && role != HasActionListRole && role != ActionListRole) {
        return Fixes::KFilePlacesModel::data(index, role);
    }

    if (index.row() < 0 || index.row() >= rowCount()) {
        return QVariant();
    }
    if (role == FavoriteIdRole) {
        return QVariant(FavoriteUtils::favoriteIdFromUrl(url(index)));
    }
    if (role == HasActionListRole) {
        return true;
    }
    if (role == ActionListRole) {
        KFileItem item(KFileItem::Unknown, KFileItem::Unknown, url(index));
        return ActionList::createListForFileItem(item);
    }
    return QVariant();
}

bool FavoritePlacesModel::trigger(int row, const QString &actionId, const QVariant &actionArgument)
{
    QModelIndex idx = index(row, 0);
    KUrl theUrl = idx.data(Fixes::KFilePlacesModel::UrlRole).value<QUrl>();
    if (actionId.isEmpty()) {
        theUrl.adjustPath(KUrl::AddTrailingSlash);
        QString rootName = idx.data(Qt::DisplayRole).toString();
        openSourceRequested("Dir", DirModel::sourceArguments(theUrl, rootName, theUrl));
        return false;
    }

    bool close;
    KFileItem item(KFileItem::Unknown, KFileItem::Unknown, theUrl);
    if (ActionList::handleFileItemAction(item, actionId, actionArgument, &close)) {
        return close;
    }

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

void FavoritePlacesModel::moveRow(int from, int to)
{
    // Simulate a move by drag'n'drop because moving the row this way
    // is the only way to get rowsMoved() signals instead of remove+insert
    QMimeData *data = mimeData(QModelIndexList() << index(from, 0));
    Q_ASSERT(data);
    if (from < to) {
        ++to;
    }
    dropMimeData(data, Qt::MoveAction, to, 0, QModelIndex());
}

bool FavoritePlacesModel::canMoveRow() const
{
    return true;
}

} // namespace Homerun

#include "favoriteplacesmodel.moc"
