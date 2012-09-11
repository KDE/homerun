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

// Local
#include <abstractsource.h>

// Qt
#include <QSortFilterProxyModel>

// KDE
#include <KDirSortFilterProxyModel>
#include <KFilePlacesModel>
#include <KUrl>

class KDirLister;
class KFileItem;

class PathModel;

/**
 * Internal
 */
class ProxyDirModel : public KDirSortFilterProxyModel
{
public:
    explicit ProxyDirModel(QObject *parent = 0);

    enum {
        FavoriteIdRole = Qt::UserRole + 1,
    };

    KFileItem itemForIndex(const QModelIndex &index) const;

    KDirLister *dirLister() const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const; // reimp
};

/**
 * Adapts KFilePlacesModel to make it usable as a Homerun favorite model
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

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const; // reimp

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
    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(QObject *rootModel READ rootModel WRITE setRootModel NOTIFY rootModelChanged)
    Q_PROPERTY(QString arguments READ arguments WRITE setArguments NOTIFY argumentsChanged)
    Q_PROPERTY(QObject *pathModel READ pathModel CONSTANT)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(QString name READ name CONSTANT)

public:
    PlacesModel(QObject *parent = 0);
    Q_INVOKABLE bool trigger(int row);

    int count() const;

    QString arguments() const;
    void setArguments(const QString &arguments);

    QString query() const;
    void setQuery(const QString &);

    QAbstractItemModel *rootModel() const;
    /**
     * Defines the root model to use. model must be a QAbstractItemModel and
     * provide a KFilePlacesModel::UrlRole returning a QUrl.
     */
    void setRootModel(QObject *model);

    QAbstractItemModel *pathModel() const;

    bool running() const;

    QString name() const;

Q_SIGNALS:
    void countChanged();
    void queryChanged();
    void rootModelChanged();
    void runningChanged(bool);
    void argumentsChanged(const QString &);
    void openSourceRequested(const QString &source);

private Q_SLOTS:
    void emitRunningChanged();

private:
    QAbstractItemModel *m_rootModel;
    ProxyDirModel *m_proxyDirModel;
    PathModel *m_pathModel;
    KUrl m_rootUrl;
    QString m_rootName;
    QString m_query;
    QString m_arguments;

    void switchToRootModel();
    void switchToDirModel();
    void openDirUrl(const KUrl &url);
};

class PlacesSource : public AbstractSource
{
public:
    PlacesSource(SourceRegistry *registry);
    QAbstractItemModel *createModel(const QString &args);
};

#endif /* PLACESMODEL_H */
