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

//- Argument parsing -----------------------------------------------------------
namespace SourceArguments
{

#define ARG_SEPARATOR ','
#define ARG_SEPARATOR_STR ","
typedef QHash<QString, QString> Hash;

QString escapeValue(const QString &src)
{
    QString dst = src;
    dst.replace('\\', "\\\\"); // must be done first
    dst.replace(ARG_SEPARATOR, "\\" ARG_SEPARATOR_STR);
    return dst;
}

static QStringList split(const QString &src)
{
    bool escaped = false;
    QStringList lst;
    QString token;
    auto it = src.constBegin(), end = src.constEnd();
    for (; it != end; ++it) {
        const QChar ch = *it;
        if (escaped) {
            escaped = false;
        } else if (ch == '\\') {
            escaped = true;
            continue;
        } else if (ch == ARG_SEPARATOR) {
            lst.append(token);
            token.clear();
            continue;
        }
        token.append(ch);
    }
    lst.append(token);
    return lst;
}

Hash parse(const QString &str)
{
    QStringList tokens = split(str);
    Hash args;
    Q_FOREACH(const QString &token, tokens) {
        int idx = token.indexOf('=');
        if (idx == -1) {
            kWarning() << "Invalid argument" << token;
            continue;
        }
        QString key = token.left(idx);
        QString value = token.mid(idx + 1);
        args.insert(key, value);
    }
    return args;
}

} // namespace

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
{
}

static QString sourceString(const KUrl &rootUrl, const QString &rootName, const KUrl &url)
{
    return QString("open PlacesModel:rootUrl=%1,rootName=%2,url=%3")
        .arg(SourceArguments::escapeValue(rootUrl.url()))
        .arg(SourceArguments::escapeValue(rootName))
        .arg(SourceArguments::escapeValue(url.url()));
}

QString PlacesModel::trigger(int row)
{
    Q_ASSERT(m_rootModel);

    QString output;
    QModelIndex sourceIndex = mapToSource(index(row, 0));
    if (sourceModel() == m_rootModel) {
        KUrl theUrl = m_rootModel->data(sourceIndex, KFilePlacesModel::UrlRole).value<QUrl>();
        theUrl.adjustPath(KUrl::AddTrailingSlash);
        QString rootName = sourceIndex.data(Qt::DisplayRole).toString();
        output = sourceString(theUrl, rootName, theUrl);
    } else {
        KFileItem item = m_proxyDirModel->itemForIndex(sourceIndex);
        if (item.isDir()) {
            output = sourceString(m_rootUrl, m_rootName, item.url());
        } else {
            item.run();
            output = "started";
        }
    }
    return output;
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

QString PlacesModel::path() const
{
    if (!m_rootModel) {
        return QString();
    }

    if (sourceModel() == m_rootModel) {
        return "/";
    }
    KUrl url = m_proxyDirModel->dirLister()->url();
    url.adjustPath(KUrl::RemoveTrailingSlash);
    QString relativePath = KUrl::relativeUrl(m_rootUrl, url);
    if (relativePath == "./") {
        return '/' + m_rootName;
    }
    return QString("/%1/%2").arg(m_rootName).arg(relativePath);
}

void PlacesModel::setPath(const QString &newPath)
{
    if (!m_rootModel) {
        kWarning() << "rootModel not set, path value ignored!";
        return;
    }

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

#include "placesmodel.moc"
