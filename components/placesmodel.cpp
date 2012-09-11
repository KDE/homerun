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

// Local
#include <pathmodel.h>
#include <sourcearguments.h>
#include <sourceregistry.h>

// KDE
#include <KDebug>
#include <KDirLister>
#include <KDirModel>
#include <KFilePlacesModel>
#include <KLocale>

static KUrl urlFromFavoriteId(const QString &favoriteId)
{
    if (!favoriteId.startsWith("place:")) {
        kWarning() << "Wrong favoriteId" << favoriteId;
        return QString();
    }
    return KUrl(favoriteId.mid(6));
}

static QString favoriteIdFromUrl(const KUrl &url)
{
    return "place:" + url.url();
}

static QString sourceString(const KUrl &rootUrl, const QString &rootName, const KUrl &url)
{
    return QString("Places:rootUrl=%1,rootName=%2,url=%3")
        .arg(SourceArguments::escapeValue(rootUrl.url()))
        .arg(SourceArguments::escapeValue(rootName))
        .arg(SourceArguments::escapeValue(url.url()));
}

//- ProxyDirModel ------------------------------------------------------
ProxyDirModel::ProxyDirModel(const KUrl &rootUrl, const QString &rootName, const KUrl &url, QObject *parent)
: KDirSortFilterProxyModel(parent)
, m_pathModel(new PathModel(this))
, m_rootUrl(rootUrl)
, m_rootName(rootName)
{
    setSourceModel(new KDirModel(this));
    setSortFoldersFirst(true);

    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "label");
    roles.insert(Qt::DecorationRole, "icon");
    roles.insert(ProxyDirModel::FavoriteIdRole, "favoriteId");
    setRoleNames(roles);

    connect(dirLister(), SIGNAL(started(KUrl)), SLOT(emitRunningChanged()));
    connect(dirLister(), SIGNAL(completed()), SLOT(emitRunningChanged()));

    initPathModel(url);

    dirLister()->openUrl(url);
}

void ProxyDirModel::initPathModel(const KUrl &openedUrl)
{
    m_pathModel->addPath(m_rootName, sourceString(m_rootUrl, m_rootName, m_rootUrl));

    QString relativePath = KUrl::relativeUrl(m_rootUrl, openedUrl);
    if (relativePath == "./") {
        return;
    }
    KUrl url = m_rootUrl;
    Q_FOREACH(const QString &token, relativePath.split('/')) {
        url.addPath(token);
        m_pathModel->addPath(token, sourceString(m_rootUrl, m_rootName, url));
    }
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

QVariant ProxyDirModel::data(const QModelIndex &index, int role) const
{
    if (role != FavoriteIdRole) {
        return QSortFilterProxyModel::data(index, role);
    }
    if (index.row() < 0 || index.row() >= rowCount()) {
        return QVariant();
    }

    KFileItem item = itemForIndex(index);
    if (item.isDir()) {
        return favoriteIdFromUrl(item.url());
    } else {
        return QString();
    }
}

int ProxyDirModel::count() const
{
    return rowCount(QModelIndex());
}

QString ProxyDirModel::name() const
{
    return dirLister()->url().fileName();
}

bool ProxyDirModel::running() const
{
    return !dirLister()->isFinished();
}

PathModel *ProxyDirModel::pathModel() const
{
    return m_pathModel;
}

void ProxyDirModel::emitRunningChanged()
{
    runningChanged(running());
}

bool ProxyDirModel::trigger(int row)
{
    bool closed = false;
    QModelIndex idx = index(row, 0);

    KFileItem item = itemForIndex(idx);
    if (item.isDir()) {
        openSourceRequested(sourceString(m_rootUrl, m_rootName, item.url()));
    } else {
        item.run();
        closed = true;
    }
    return closed;
}

//- FavoritePlacesModel ------------------------------------------------
FavoritePlacesModel::FavoritePlacesModel(QObject *parent)
: KFilePlacesModel(parent)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "label");
    roles.insert(Qt::DecorationRole, "icon");
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
    openSourceRequested(sourceString(theUrl, rootName, theUrl));

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

//- PlacesSource -------------------------------------------------------
PlacesSource::PlacesSource(SourceRegistry *registry)
: AbstractSource(registry)
{}

QAbstractItemModel *PlacesSource::createModel(const QString &str)
{
    SourceArguments::Hash args = SourceArguments::parse(str);
    KUrl rootUrl = args.value("rootUrl");
    QString rootName = args.value("rootName");
    KUrl url = args.value("url");

    if (!rootUrl.isValid()) {
        return registry()->favoriteModel("place");
    }

    if (rootName.isEmpty()) {
        rootName = rootUrl.fileName();
        if (rootName.isEmpty()) {
            rootName = rootUrl.prettyUrl();
        }
    }
    if (!url.isValid()) {
        url = rootUrl;
    }
    url.adjustPath(KUrl::RemoveTrailingSlash);

    return new ProxyDirModel(rootUrl, rootName, url, registry());
}

#include "placesmodel.moc"
