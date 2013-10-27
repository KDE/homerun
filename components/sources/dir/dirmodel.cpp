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
// Self
#include <dirmodel.h>

// Local
#include <dirconfigurationwidget.h>
#include <favoriteutils.h>

// libhomerun
#include <actionlist.h>
#include <pathmodel.h>

// KDE
#include <KConfigGroup>
#include <KDebug>
#include <KDirModel>
#include <KDirLister>

// Qt
#include <QDir>

namespace Homerun
{

static const char *SOURCE_ID = "Dir";

static inline KFileItem itemForIndex(const QModelIndex &index)
{
    return index.data(KDirModel::FileItemRole).value<KFileItem>();
}

//- DirModel ------------------------------------------------------
QVariantMap DirModel::sourceArguments(const KUrl &rootUrl, const QString &rootName, const KUrl &url)
{
    QVariantMap args;
    args.insert("rootUrl", rootUrl.url());
    args.insert("rootName", rootName);
    args.insert("url", url.url());
    return args;
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
    roles.insert(DirModel::HasActionListRole, "hasActionList");
    roles.insert(DirModel::ActionListRole, "actionList");
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
    QVariantMap args = sourceArguments(m_rootUrl, m_rootName, m_rootUrl);
    m_pathModel->addPath(m_rootName, SOURCE_ID, args);

    KUrl rootUrl = m_rootUrl;
    // Needed for KUrl::relativeUrl
    rootUrl.adjustPath(KUrl::AddTrailingSlash);
    QString relativePath = KUrl::relativeUrl(rootUrl, openedUrl);
    if (relativePath == "./") {
        return;
    }
    KUrl url = m_rootUrl;
    Q_FOREACH(const QString &token, relativePath.split('/')) {
        if (token.isEmpty()) {
            // Just in case relativePath ends with '/'
            continue;
        }
        url.addPath(token);
        args["url"] = url.url();
        m_pathModel->addPath(token, SOURCE_ID, args);
    }
}

KDirLister *DirModel::dirLister() const
{
    return static_cast<KDirModel *>(sourceModel())->dirLister();
}

QVariant DirModel::data(const QModelIndex &index, int role) const
{
    KFileItem item = qvariant_cast<KFileItem>(QSortFilterProxyModel::data(index, KDirModel::FileItemRole));

    if (role == Qt::DecorationRole && !item.isFinalIconKnown()) {
        item.determineMimeType();
    }

    if (role != FavoriteIdRole && role != HasActionListRole && role != ActionListRole) {
        return QSortFilterProxyModel::data(index, role);
    }
    if (index.row() < 0 || index.row() >= rowCount()) {
        return QVariant();
    }
    if (role == HasActionListRole) {
        return true;
    }
    if (role == FavoriteIdRole) {
        if (item.isDir()) {
            return FavoriteUtils::favoriteIdFromUrl(item.url());
        } else {
            return QString();
        }
    } else if (role == ActionListRole) {
        return ActionList::createListForFileItem(item);
    }
    // Never reached
    return QVariant();
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

bool DirModel::trigger(int row, const QString &actionId, const QVariant &actionArg)
{
    QModelIndex idx = index(row, 0);
    KFileItem item = itemForIndex(idx);

    if (actionId.isEmpty()) {
        if (item.isDir()) {
            openSourceRequested(SOURCE_ID, sourceArguments(m_rootUrl, m_rootName, item.url()));
        } else {
            item.run();
        }
    }

    bool close;
    ActionList::handleFileItemAction(item, actionId, actionArg, &close);

    return false;
}

//- DirSource -------------------------------------------------------
DirSource::DirSource(QObject *parent)
: AbstractSource(parent)
{}

QAbstractItemModel *DirSource::createModelFromArguments(const QVariantMap &args)
{
    KUrl rootUrl = args.value("rootUrl").toString();
    QString rootName = args.value("rootName").toString();
    KUrl url = args.value("url").toString();
    return createModel(rootUrl, rootName, url);
}

QAbstractItemModel *DirSource::createModelFromConfigGroup(const KConfigGroup &group)
{
    KUrl rootUrl = group.readPathEntry("rootUrl", QDir::homePath());
    QString rootName = group.readEntry("rootName", QString());
    return createModel(rootUrl, rootName, KUrl());
}

QAbstractItemModel *DirSource::createModel(const KUrl &rootUrl_, const QString &rootName_, const KUrl &url_)
{
    KUrl rootUrl = rootUrl_;
    QString rootName = rootName_;
    KUrl url = url_;
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

    DirModel *model = new DirModel;
    model->init(rootUrl, rootName, url);
    return model;
}

bool DirSource::isConfigurable() const
{
    return true;
}

SourceConfigurationWidget *DirSource::createConfigurationWidget(const KConfigGroup &group)
{
    return new DirConfigurationWidget(group);
}

} // namespace Homerun

#include <dirmodel.moc>
