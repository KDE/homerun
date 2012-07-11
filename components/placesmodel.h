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
#ifndef PLACESMODEL_H
#define PLACESMODEL_H

#include <QSortFilterProxyModel>

#include <KDirSortFilterProxyModel>
#include <KFilePlacesModel>
#include <KUrl>

class KDirLister;
class KFileItem;

/**
 * Internal
 */
class ProxyDirModel : public KDirSortFilterProxyModel
{
public:
    explicit ProxyDirModel(QObject *parent = 0);

    KFileItem itemForIndex(const QModelIndex &index) const;

    KDirLister *dirLister() const;
};

/**
 * Adapts KFilePlacesModel to make it usable as a SAL favorite model
 */
class FavoritePlacesModel : public KFilePlacesModel
{
    Q_OBJECT
    Q_PROPERTY(QString favoritePrefix READ favoritePrefix CONSTANT)

public:
    enum {
        FavoriteIdRole = Qt::UserRole + 1,
    };

    FavoritePlacesModel(QObject *parent = 0);

    Q_INVOKABLE bool isFavorite(const QString &favoriteId) const;
    Q_INVOKABLE void addFavorite(const QString &favoriteId);
    Q_INVOKABLE void removeFavorite(const QString &favoriteId);

    QString favoritePrefix() const;

private:
    QModelIndex indexForFavoriteId(const QString &favoriteId) const;
};

/**
 * A model which can be used to navigate from favorite places
 */
class PlacesModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(QObject *rootModel READ rootModel WRITE setRootModel NOTIFY rootModelChanged)

public:
    PlacesModel(QObject *parent = 0);
    Q_INVOKABLE bool trigger(int row);

    int count() const;

    QString path() const;
    void setPath(const QString &path);

    QString filter() const;
    void setFilter(const QString &filter);

    FavoritePlacesModel *rootModel() const;
    void setRootModel(QObject *model);

Q_SIGNALS:
    void countChanged();
    void filterChanged();
    void rootModelChanged();
    void pathChanged(const QString &);

private:
    FavoritePlacesModel *m_rootModel;
    ProxyDirModel *m_proxyDirModel;
    KUrl m_rootUrl;
    QString m_rootName;
    QString m_filter;

    void switchToRootModel();
    void switchToDirModel();
    void openDirUrl(const KUrl &url);
};

#endif /* PLACESMODEL_H */
