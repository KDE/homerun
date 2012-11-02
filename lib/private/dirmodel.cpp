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
// Self
#include <dirmodel.h>

// Local
#include <dirconfigurationwidget.h>
#include <pathmodel.h>
#include <placesmodel.h>
#include <sourceid.h>

// KDE
#include <KDebug>
#include <KDirModel>
#include <KDirLister>

// Qt
#include <QDir>

namespace Homerun
{

static inline KFileItem itemForIndex(const QModelIndex &index)
{
    return index.data(KDirModel::FileItemRole).value<KFileItem>();
}

//- DirModel ------------------------------------------------------
QString DirModel::sourceString(const KUrl &rootUrl, const QString &rootName, const KUrl &url)
{
    SourceId sourceId;
    sourceId.setName("Dir");
    sourceId.arguments()
        .add("rootUrl", rootUrl.url())
        .add("rootName", rootName)
        .add("url", url.url());
    return sourceId.toString();
}


DirModel::DirModel(QObject *parent)
: KDirSortFilterProxyModel(parent)
, m_pathModel(new PathModel(this))
{
    setSourceModel(new KDirModel(this));
    setSortFoldersFirst(true);

    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::DecorationRole, "decoration");
    roles.insert(DirModel::FavoriteIdRole, "favoriteId");
    setRoleNames(roles);

    dirLister()->setDelayedMimeTypes(true);
    connect(dirLister(), SIGNAL(started(KUrl)), SLOT(emitRunningChanged()));
    connect(dirLister(), SIGNAL(completed()), SLOT(emitRunningChanged()));
}

void DirModel::init(const KUrl &rootUrl, const QString &rootName, const KUrl &url)
{
    m_rootUrl = rootUrl;
    m_rootName = rootName;
    initPathModel(url);
    dirLister()->openUrl(url);
}

void DirModel::initPathModel(const KUrl &openedUrl)
{
    SourceId sourceId;
    sourceId.setName("Dir");
    sourceId.arguments()
        .add("rootUrl", m_rootUrl.url())
        .add("rootName", m_rootName)
        .add("url", m_rootUrl.url());

    m_pathModel->addPath(m_rootName, sourceId.toString());

    QString relativePath = KUrl::relativeUrl(m_rootUrl, openedUrl);
    if (relativePath == "./") {
        return;
    }
    KUrl url = m_rootUrl;
    Q_FOREACH(const QString &token, relativePath.split('/')) {
        url.addPath(token);
        sourceId.arguments()["url"] = url.url();
        m_pathModel->addPath(token, sourceId.toString());
    }
}

KDirLister *DirModel::dirLister() const
{
    return static_cast<KDirModel *>(sourceModel())->dirLister();
}

QVariant DirModel::data(const QModelIndex &index, int role) const
{
    if (role != FavoriteIdRole) {
        return QSortFilterProxyModel::data(index, role);
    }
    if (index.row() < 0 || index.row() >= rowCount()) {
        return QVariant();
    }

    KFileItem item = itemForIndex(index);
    if (item.isDir()) {
        return FavoritePlacesModel::favoriteIdFromUrl(item.url());
    } else {
        return QString();
    }
}

int DirModel::count() const
{
    return rowCount(QModelIndex());
}

QString DirModel::name() const
{
    return m_rootName;
}

bool DirModel::running() const
{
    return !dirLister()->isFinished();
}

PathModel *DirModel::pathModel() const
{
    return m_pathModel;
}

QString DirModel::query() const
{
    return filterRegExp().pattern();
}

void DirModel::setQuery(const QString &value)
{
    if (value == query()) {
        return;
    }
    setFilterRegExp(QRegExp(value, Qt::CaseInsensitive));
    queryChanged(value);
}

void DirModel::emitRunningChanged()
{
    runningChanged(running());
}

bool DirModel::trigger(int row)
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

//- DirSource -------------------------------------------------------
DirSource::DirSource(QObject *parent)
: AbstractSource(parent)
{}

QAbstractItemModel *DirSource::createModel(const SourceArguments &args)
{
    KUrl rootUrl = args.value("rootUrl");
    QString rootName = args.value("rootName");
    KUrl url = args.value("url");

    if (!rootUrl.isValid()) {
        rootUrl = KUrl::fromPath(QDir::homePath());
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

    DirModel *model = new DirModel;
    model->init(rootUrl, rootName, url);
    return model;
}

bool DirSource::isConfigurable() const
{
    return true;
}

SourceConfigurationWidget *DirSource::createConfigurationWidget(const SourceArguments &args)
{
    return new DirConfigurationWidget(args);
}

} // namespace Homerun

#include <dirmodel.moc>
