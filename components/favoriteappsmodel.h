/*
    Copyright 2009 Ivan Cukic <ivan.cukic+kde@gmail.com>
    Copyright 2010 Marco Martin <notmart@gmail.com>
    Copyright 2012 Aurélien Gâteau <agateau@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef FAVORITEAPPSMODEL_H
#define FAVORITEAPPSMODEL_H

#include <QAbstractListModel>

#include <KService>
#include <KSharedConfig>

struct FavoriteInfo
{
    int rank;
    KService::Ptr service;
};

class FavoriteAppsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

    Q_PROPERTY(QString favoritePrefix READ favoritePrefix CONSTANT)

public:
    FavoriteAppsModel(QObject *parent = 0);
    ~FavoriteAppsModel();

    enum {
        FavoriteIdRole = Qt::UserRole + 1
    };

    int count() const;
    QString name() const;
    QString favoritePrefix() const;

    void setConfig(const KSharedConfig::Ptr &);

    int rowCount(const QModelIndex & = QModelIndex()) const;
    QVariant data(const QModelIndex &, int role = Qt::DisplayRole) const;

    Q_INVOKABLE bool isFavorite(const QString &favoriteId) const;
    Q_INVOKABLE void addFavorite(const QString &favoriteId);
    Q_INVOKABLE void removeFavorite(const QString &favoriteId);

    Q_INVOKABLE bool trigger(int row);

Q_SIGNALS:
    void countChanged();

private:
    KSharedConfig::Ptr m_config;
    QList<FavoriteInfo> m_favoriteList;

    int rowForFavoriteId(const QString &favoriteId) const;
};

#endif // FAVORITEAPPSMODEL_H
