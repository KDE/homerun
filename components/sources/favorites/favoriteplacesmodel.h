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
#ifndef PLACESMODEL_H
#define PLACESMODEL_H

// Local
#include <fileplacesmodel.h>

// Qt

// KDE

namespace Homerun {

/**
 * Adapts KFilePlacesModel to make it usable as a Homerun favorite model
 */
class FavoritePlacesModel : public Fixes::KFilePlacesModel
{
    Q_OBJECT
    Q_PROPERTY(QString favoritePrefix READ favoritePrefix CONSTANT)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(bool canMoveRow READ canMoveRow CONSTANT)

public:
    enum {
        FavoriteIdRole = Qt::UserRole + 1,
        HasActionListRole,
        ActionListRole,
    };

    FavoritePlacesModel(QObject *parent = 0);

    Q_INVOKABLE bool isFavorite(const QString &favoriteId) const;
    Q_INVOKABLE void addFavorite(const QString &favoriteId);
    Q_INVOKABLE void removeFavorite(const QString &favoriteId);
    Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &actionArg);
    Q_INVOKABLE void moveRow(int from, int to);

    QString favoritePrefix() const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const; // reimp

    QString name() const;

    int count() const;

    bool canMoveRow() const;

Q_SIGNALS:
    void openSourceRequested(const QString &sourceId, const QVariantMap &sourceArguments);
    void countChanged();

private:
    QModelIndex indexForFavoriteId(const QString &favoriteId) const;
};

} // namespace Homerun

#endif /* PLACESMODEL_H */
