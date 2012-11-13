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

// Qt

// KDE
#include <KFilePlacesModel>

namespace Homerun {

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

    static KUrl urlFromFavoriteId(const QString &favoriteId);
    static QString favoriteIdFromUrl(const KUrl &url);

Q_SIGNALS:
    void openSourceRequested(const QString &sourceId, const QVariantMap &sourceArguments);
    void countChanged();

private:
    QModelIndex indexForFavoriteId(const QString &favoriteId) const;
};

} // namespace Homerun

#endif /* PLACESMODEL_H */
