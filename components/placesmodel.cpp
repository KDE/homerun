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

//- FavoritePlacesModel ------------------------------------------------
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

//- PlacesModel --------------------------------------------------------
PlacesModel::PlacesModel(QObject *parent)
: QSortFilterProxyModel(parent)
, m_rootModel(0)
, m_proxyDirModel(new ProxyDirModel(this))
, m_pathModel(new PathModel(this))
{
    connect(m_proxyDirModel->dirLister(), SIGNAL(started(KUrl)), SLOT(emitRunningChanged()));
    connect(m_proxyDirModel->dirLister(), SIGNAL(completed()), SLOT(emitRunningChanged()));
}

static QString sourceString(const KUrl &rootUrl, const QString &rootName, const KUrl &url)
{
    return QString("Places:rootUrl=%1,rootName=%2,url=%3")
        .arg(SourceArguments::escapeValue(rootUrl.url()))
        .arg(SourceArguments::escapeValue(rootName))
        .arg(SourceArguments::escapeValue(url.url()));
}

bool PlacesModel::trigger(int row)
{
    Q_ASSERT(m_rootModel);

    bool closed = false;
    QModelIndex sourceIndex = mapToSource(index(row, 0));
    if (sourceModel() == m_rootModel) {
        KUrl theUrl = m_rootModel->data(sourceIndex, KFilePlacesModel::UrlRole).value<QUrl>();
        theUrl.adjustPath(KUrl::AddTrailingSlash);
        QString rootName = sourceIndex.data(Qt::DisplayRole).toString();
        openSourceRequested(sourceString(theUrl, rootName, theUrl));
    } else {
        KFileItem item = m_proxyDirModel->itemForIndex(sourceIndex);
        if (item.isDir()) {
            openSourceRequested(sourceString(m_rootUrl, m_rootName, item.url()));
        } else {
            item.run();
            closed = true;
        }
    }
    return closed;
}

int PlacesModel::count() const
{
    int c = rowCount(QModelIndex());
    return c;
}

void PlacesModel::switchToRootModel()
{
    Q_ASSERT(m_rootModel);
    setSourceModel(m_rootModel);

    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "label");
    roles.insert(Qt::DecorationRole, "icon");
    roles.insert(FavoritePlacesModel::FavoriteIdRole, "favoriteId");
    setRoleNames(roles);
    m_pathModel->clear();
}

void PlacesModel::switchToDirModel()
{
    Q_ASSERT(m_rootModel);
    setSourceModel(m_proxyDirModel);

    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "label");
    roles.insert(Qt::DecorationRole, "icon");
    roles.insert(ProxyDirModel::FavoriteIdRole, "favoriteId");
    setRoleNames(roles);
}

QAbstractItemModel *PlacesModel::pathModel() const
{
    return m_pathModel;
}

QString PlacesModel::arguments() const
{
    return m_arguments;
}

void PlacesModel::setArguments(const QString &str)
{
    if (m_arguments == str) {
        return;
    }

    SourceArguments::Hash args = SourceArguments::parse(str);
    KUrl rootUrl = args.value("rootUrl");
    QString rootName = args.value("rootName");
    KUrl url = args.value("url");

    if (!rootUrl.isValid()) {
        // No rootUrl, going back to place list
        switchToRootModel();

        m_arguments = str;
        argumentsChanged(str);
        return;
    }

    // Opening a root
    if (rootName.isEmpty()) {
        kWarning() << "Invalid name passed as 'rootName' argument in" << str;
    }
    if (!url.isValid()) {
        kWarning() << "Invalid url passed as 'url' argument in" << str;
        return;
    }

    if (sourceModel() == m_rootModel) {
        switchToDirModel();
    }
    m_rootUrl = rootUrl;
    m_rootName = rootName;
    openDirUrl(url);

    m_arguments = str;
    argumentsChanged(str);
}

QString PlacesModel::query() const
{
    return m_query;
}

void PlacesModel::setQuery(const QString& query)
{
    if (m_query == query) {
        return;
    }
    m_query = query;
    QRegExp rx(query, Qt::CaseInsensitive, QRegExp::FixedString);
    setFilterRegExp(rx);
    filterChanged();

}

void PlacesModel::openDirUrl(const KUrl &_url)
{
    KUrl url = _url;
    url.adjustPath(KUrl::RemoveTrailingSlash);
    m_proxyDirModel->dirLister()->openUrl(url);

    // Update m_pathModel
    m_pathModel->clear();
    m_pathModel->addPath(m_rootName, sourceString(m_rootUrl, m_rootName, m_rootUrl));

    QString relativePath = KUrl::relativeUrl(m_rootUrl, url);
    if (relativePath == "./") {
        return;
    }
    url = m_rootUrl;
    Q_FOREACH(const QString &token, relativePath.split('/')) {
        url.addPath(token);
        m_pathModel->addPath(token, sourceString(m_rootUrl, m_rootName, url));
    }
}

QAbstractItemModel *PlacesModel::rootModel() const
{
    return m_rootModel;
}

void PlacesModel::setRootModel(QObject *obj)
{
    if (obj) {
        m_rootModel = qobject_cast<QAbstractItemModel *>(obj);
        Q_ASSERT(m_rootModel);
        switchToRootModel();
    } else {
        m_rootModel = 0;
    }
    rootModelChanged();
}

bool PlacesModel::running() const
{
    if (sourceModel() == m_rootModel) {
        return false;
    } else {
        return !m_proxyDirModel->dirLister()->isFinished();
    }
}

void PlacesModel::emitRunningChanged()
{
    if (sourceModel() == m_rootModel) {
        return;
    }
    runningChanged(running());
}

QString PlacesModel::name() const
{
    return i18n("Favorite Places");
}

//- PlacesSource -------------------------------------------------------
PlacesSource::PlacesSource(SourceRegistry *registry)
: AbstractSource(registry)
{}

QAbstractItemModel *PlacesSource::createModel(const QString &args)
{
    PlacesModel *model = new PlacesModel(registry());
    model->setRootModel(registry()->favoriteModel("place"));
    model->setArguments(args);
    return model;
}

#include "placesmodel.moc"
