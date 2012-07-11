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
#include <KDirLister>
#include <KDirModel>
#include <KFilePlacesModel>

//- ProxyDirModel ------------------------------------------------------
ProxyDirModel::ProxyDirModel(QObject *parent)
: KDirSortFilterProxyModel(parent)
{
    setSourceModel(new KDirModel(this));
    setSortFoldersFirst(true);
}

KFileItem ProxyDirModel::itemForIndex(const QModelIndex &index) const
{
    const QModelIndex sourceIndex = mapToSource(index);
    return static_cast<KDirModel *>(sourceModel())->itemForIndex(sourceIndex);
}

KDirLister *ProxyDirModel::dirLister() const
{
    return static_cast<KDirModel *>(sourceModel())->dirLister();
}

//- FavoritePlacesModel ------------------------------------------------
static KUrl urlFromFavoriteId(const QString &favoriteId)
{
    if (!favoriteId.startsWith("place:")) {
        kWarning() << "Wrong favoriteId" << favoriteId;
        return QString();
    }
    return KUrl(favoriteId.mid(6));
}

FavoritePlacesModel::FavoritePlacesModel(QObject *parent)
: KFilePlacesModel(parent)
{}

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

//- PlacesModel --------------------------------------------------------
PlacesModel::PlacesModel(QObject *parent)
: QSortFilterProxyModel(parent)
, m_rootModel(new FavoritePlacesModel(this))
, m_proxyDirModel(new ProxyDirModel(this))
{
    switchToRootModel();
}

bool PlacesModel::trigger(int row)
{
    bool close = false;
    QModelIndex sourceIndex = mapToSource(index(row, 0));
    if (sourceModel() == m_rootModel) {
        KUrl theUrl = m_rootModel->url(sourceIndex);
        switchToDirModel();

        m_rootUrl = theUrl;
        m_rootUrl.adjustPath(KUrl::AddTrailingSlash);
        m_rootName = "/" + sourceIndex.data(Qt::DisplayRole).toString();
        openDirUrl(theUrl);
    } else {
        KFileItem item = m_proxyDirModel->itemForIndex(sourceIndex);
        if (item.isDir()) {
            openDirUrl(item.url());
        } else {
            item.run();
            close = true;
        }
    }
    return close;
}

int PlacesModel::count() const
{
    int c = rowCount(QModelIndex());
    return c;
}

void PlacesModel::switchToRootModel()
{
    setSourceModel(m_rootModel);

    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "label");
    roles.insert(Qt::DecorationRole, "icon");
    setRoleNames(roles);
}

void PlacesModel::switchToDirModel()
{
    setSourceModel(m_proxyDirModel);

    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "label");
    roles.insert(Qt::DecorationRole, "icon");
    setRoleNames(roles);
}

QString PlacesModel::path() const
{
    if (sourceModel() == m_rootModel) {
        return "/";
    }
    KUrl url = m_proxyDirModel->dirLister()->url();
    url.adjustPath(KUrl::RemoveTrailingSlash);
    QString relativePath = KUrl::relativeUrl(m_rootUrl, url);
    if (relativePath == "./") {
        return m_rootName;
    }
    return QString("%1/%2").arg(m_rootName).arg(relativePath);
}

void PlacesModel::setPath(const QString &newPath)
{
    if (newPath == "/") {
        switchToRootModel();
    } else {
        KUrl url = m_rootUrl;
        // Ugly: skip the first path token: it is the place name
        url.addPath(newPath.section('/', 2));
        openDirUrl(url);
    }
    pathChanged(path());
}

QString PlacesModel::filter() const
{
    return m_filter;
}

void PlacesModel::setFilter(const QString& filter)
{
    if (m_filter == filter) {
        return;
    }
    m_filter = filter;
    QRegExp rx(filter, Qt::CaseInsensitive, QRegExp::FixedString);
    setFilterRegExp(rx);
    filterChanged();

}

void PlacesModel::openDirUrl(const KUrl &url)
{
    m_proxyDirModel->dirLister()->openUrl(url);
    pathChanged(path());
}

/*
QVariant PlacesModel::data(const QModelIndex& index, int role) const
{
    if (role != FavoriteActionRole) {
        return QSortFilterProxyModel::data(index, role);
    }
    if (index.row() < 0 || index.row() >= rowCount()) {
        return QVariant();
    }
    QModelIndex sourceIndex = mapToSource(index);
    QString value;
    if (sourceModel() == m_rootModel) {
        value = m_rootModel->isDevice(sourceIndex) ? "" : "remove";
    } else {
        KFileItem item = m_proxyDirModel->itemForIndex(sourceIndex);
        value = item.isDir() ? "add" : "";
    }
    return value;
}
*/

FavoritePlacesModel *PlacesModel::rootModel() const
{
    return m_rootModel;
}

#include "placesmodel.moc"
