/*
 *   Copyright 2013 Eike Hein <hein@kde.org>
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef COMBINEDPOWERSESSIONMODEL_H
#define COMBINEDPOWERSESSIONMODEL_H

#include <abstractsource.h>

#include <QAbstractListModel>

#include <KConfigGroup>

namespace Homerun {

class PowerModel;
class SessionModel;
class CombinedPowerSessionModel;

class PowerSessionFavoritesModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(bool canMoveRow READ canMoveRow CONSTANT)

    public:
        PowerSessionFavoritesModel(const KConfigGroup &group, CombinedPowerSessionModel *parent = 0);
        ~PowerSessionFavoritesModel();

        int count() const;
        int rowCount(const QModelIndex& parent = QModelIndex()) const;

        bool canMoveRow() const;

        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

        Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &/*actionArgument*/);

        Q_INVOKABLE void moveRow(int from, int to);

        void addFavorite(const QModelIndex &index);
        void removeFavorite(const QModelIndex &index);
        bool isFavorite(const QModelIndex &index);

        QString name() const;

    signals:
        void countChanged();

    private:
        void readConfig();
        void writeConfig();

        KConfigGroup m_configGroup;

        QList<QPersistentModelIndex> m_favorites;
        CombinedPowerSessionModel* m_combinedPowerSessionModel;
};

class CombinedPowerSessionModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(bool showFavoritesActions READ showFavoritesActions WRITE setShowFavoritesActions)

    public:
        enum Roles {
            FavoriteIdRole = Qt::UserRole + 1,
            HasActionListRole,
            ActionListRole
        };

        CombinedPowerSessionModel(const KConfigGroup &group, QObject *parent = 0);
        ~CombinedPowerSessionModel();

        int count() const;
        int rowCount(const QModelIndex& parent = QModelIndex()) const;

        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

        Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &/*actionArgument*/);

        bool showFavoritesActions() const;
        void setShowFavoritesActions(bool show);
        Q_INVOKABLE QObject *favoritesModel() const;

        QString name() const;

    signals:
        void countChanged();

    private:
        SessionModel *m_sessionModel;
        PowerModel *m_powerModel;

        QHash<QString, QString> m_favoriteIdMapping;
        PowerSessionFavoritesModel* m_favoritesModel;
        bool m_showFavoritesActions;
};

class CombinedPowerSessionSource : public AbstractSource
{
    public:
        CombinedPowerSessionSource(QObject *parent);
        QAbstractItemModel *createModelFromConfigGroup(const KConfigGroup &group);
};

}

#endif
