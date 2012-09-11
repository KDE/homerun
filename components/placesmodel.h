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
class DirModel : public KDirSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString name READ name CONSTANT)

    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(QObject *pathModel READ pathModel CONSTANT)
    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
public:
    DirModel(const KUrl &rootUrl, const QString &rootName, const KUrl &url, QObject *parent);

    enum {
        FavoriteIdRole = Qt::UserRole + 1,
    };

    Q_INVOKABLE bool trigger(int row);

    KFileItem itemForIndex(const QModelIndex &index) const;

    KDirLister *dirLister() const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const; // reimp

    PathModel *pathModel() const;

    int count() const;

    QString name() const;

    bool running() const;

    QString query() const;

    void setQuery(const QString &query);

Q_SIGNALS:
    void countChanged();
    void runningChanged(bool);
    void openSourceRequested(const QString &source);
    void queryChanged(const QString &);

private Q_SLOTS:
    void emitRunningChanged();

private:
    PathModel *m_pathModel;
    KUrl m_rootUrl;
    QString m_rootName;

    void initPathModel(const KUrl &url);
};

/**
 * Adapts KFilePlacesModel to make it usable as a Homerun favorite model
 */
class FavoritePlacesModel : public KFilePlacesModel
{
    Q_OBJECT
    Q_PROPERTY(QString favoritePrefix READ favoritePrefix CONSTANT)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString name READ name CONSTANT)

public:
    enum {
        FavoriteIdRole = Qt::UserRole + 1,
    };

    FavoritePlacesModel(QObject *parent = 0);

    Q_INVOKABLE bool isFavorite(const QString &favoriteId) const;
    Q_INVOKABLE void addFavorite(const QString &favoriteId);
    Q_INVOKABLE void removeFavorite(const QString &favoriteId);
    Q_INVOKABLE bool trigger(int row);

    QString favoritePrefix() const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const; // reimp

    QString name() const;

    int count() const;

Q_SIGNALS:
    void openSourceRequested(const QString &source);
    void countChanged();

private:
    QModelIndex indexForFavoriteId(const QString &favoriteId) const;
};

class DirSource : public AbstractSource
{
public:
    DirSource(SourceRegistry *registry);
    QAbstractItemModel *createModel(const QString &args);
};

#endif /* PLACESMODEL_H */
