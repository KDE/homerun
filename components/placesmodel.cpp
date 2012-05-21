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

PlacesModel::PlacesModel(QObject *parent)
: QSortFilterProxyModel(parent)
, m_placesModel(new KFilePlacesModel(this))
, m_dirModel(new KDirModel(this))
{
    setDynamicSortFilter(true);
    switchToPlacesModel();
}

void PlacesModel::run(int row)
{
    QModelIndex sourceIndex = mapToSource(index(row, 0));
    if (sourceModel() == m_placesModel) {
        KUrl theUrl = m_placesModel->url(sourceIndex);
        switchToDirModel();

        m_rootUrl = theUrl;
        m_rootUrl.adjustPath(KUrl::AddTrailingSlash);
        m_rootName = "/" + sourceIndex.data(Qt::DisplayRole).toString();
        openDirUrl(theUrl);
    } else {
        KFileItem item = m_dirModel->itemForIndex(sourceIndex);
        if (item.isDir()) {
            openDirUrl(item.url());
        } else {
            item.run();
        }
    }
}

int PlacesModel::count() const
{
    int c = rowCount(QModelIndex());
    return c;
}

void PlacesModel::switchToPlacesModel()
{
    setSourceModel(m_placesModel);
    sort(-1, Qt::AscendingOrder);

    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "label");
    roles.insert(Qt::DecorationRole, "icon");
    setRoleNames(roles);
}

void PlacesModel::switchToDirModel()
{
    setSourceModel(m_dirModel);
    sort(0, Qt::AscendingOrder);

    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "label");
    roles.insert(Qt::DecorationRole, "icon");
    setRoleNames(roles);
}

QString PlacesModel::path() const
{
    if (sourceModel() == m_placesModel) {
        return "/";
    }
    KUrl url = m_dirModel->dirLister()->url();
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
        switchToPlacesModel();
    } else {
        KUrl url = m_rootUrl;
        // Ugly: skip the first path token: it is the place name
        url.addPath(newPath.section('/', 2));
        openDirUrl(url);
    }
    pathChanged(path());
}

void PlacesModel::openDirUrl(const KUrl &url)
{
    m_dirModel->dirLister()->openUrl(url);
    pathChanged(path());
}

#include "placesmodel.moc"
