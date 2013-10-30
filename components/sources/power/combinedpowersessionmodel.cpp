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

#include "combinedpowersessionmodel.h"
#include "actionlist.h"
#include "powermodel.h"
#include "sessionmodel.h"

#include <KIcon>
#include <KLocale>

namespace Homerun {

PowerSessionFavoritesModel::PowerSessionFavoritesModel(const KConfigGroup &group, CombinedPowerSessionModel *parent) : QAbstractListModel(parent)
, m_configGroup(group)
, m_combinedPowerSessionModel(parent)
{
    setRoleNames(m_combinedPowerSessionModel->roleNames());

    readConfig();
}

PowerSessionFavoritesModel::~PowerSessionFavoritesModel()
{
}

void PowerSessionFavoritesModel::readConfig()
{
    beginResetModel();

    m_favorites.clear();

    QStringList favoriteIds = m_configGroup.readEntry("Favorites", QList<QString>() << "logout" << "restart" << "shutdown");

    const QModelIndex &start = m_combinedPowerSessionModel->index(0, 0);

    foreach(const QString &favoriteId, favoriteIds) {
        QModelIndexList indices = m_combinedPowerSessionModel->match(start, CombinedPowerSessionModel::FavoriteIdRole,
            favoriteId, 1, Qt::MatchExactly);

        if (indices.count()) {
            m_favorites.append(QPersistentModelIndex(indices.at(0)));
        }
    }

    endResetModel();

    emit countChanged();

    writeConfig();
}

void PowerSessionFavoritesModel::writeConfig()
{
    QStringList favoriteIds;

    foreach(const QPersistentModelIndex &index, m_favorites) {
        favoriteIds << index.data(CombinedPowerSessionModel::FavoriteIdRole).toString();
    }

    m_configGroup.writeEntry("Favorites", favoriteIds);
    m_configGroup.config()->sync();
}

int PowerSessionFavoritesModel::count() const
{
    return rowCount();
}

int PowerSessionFavoritesModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_favorites.count();
}

bool PowerSessionFavoritesModel::canMoveRow() const
{
    return true;
}

QVariant PowerSessionFavoritesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() > m_favorites.count() - 1) {
        return QVariant();
    }

    return m_favorites.at(index.row()).data(role);
}

bool PowerSessionFavoritesModel::trigger(int row, const QString &actionId, const QVariant &)
{
    if (row > m_favorites.count() - 1) {
        return false;
    }

    return m_combinedPowerSessionModel->trigger(m_favorites.at(row).row(), actionId, QString());
}

void PowerSessionFavoritesModel::moveRow(int from, int to)
{
    beginMoveRows(QModelIndex(), from, from, QModelIndex(), to > from ? to + 1 : to);

    m_favorites.move(from, to);

    endMoveRows();

    writeConfig();
}

void PowerSessionFavoritesModel::addFavorite(const QModelIndex &index)
{
    QString favoriteId = index.data(CombinedPowerSessionModel::FavoriteIdRole).toString();

    if (favoriteId.isEmpty()) {
        return;
    }

    beginInsertRows(QModelIndex(), m_favorites.count(), m_favorites.count());

    m_favorites.append(QPersistentModelIndex(index));

    endInsertRows();

    emit countChanged();

    writeConfig();
}

void PowerSessionFavoritesModel::removeFavorite(const QModelIndex &index)
{
    int row = m_favorites.indexOf(index);

    if (row != -1) {
        beginRemoveRows(QModelIndex(), row, row);

        m_favorites.removeAt(row);

        endRemoveRows();

        emit countChanged();

        writeConfig();
    }
}

bool PowerSessionFavoritesModel::isFavorite(const QModelIndex &index)
{
    return m_favorites.contains(index);
}

QString PowerSessionFavoritesModel::name() const
{
    return m_combinedPowerSessionModel->name();
}

CombinedPowerSessionModel::CombinedPowerSessionModel(const KConfigGroup &group, QObject *parent) : QAbstractListModel(parent)
, m_sessionModel(new SessionModel(this))
, m_powerModel(new PowerModel(this))
, m_showFavoritesActions(false)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::DecorationRole, "decoration");
    roles.insert(HasActionListRole, "hasActionList");
    roles.insert(ActionListRole, "actionList");

    setRoleNames(roles);

    m_favoriteIdMapping["system-lock-screen"] = "lock-screen";
    m_favoriteIdMapping["system-log-out"] = "logout";
    m_favoriteIdMapping["system-switch-user"] = "switch-user";
    m_favoriteIdMapping["system-suspend"] = "suspend";
    m_favoriteIdMapping["system-suspend-hibernate"] = "hibernate";
    m_favoriteIdMapping["system-reboot"] = "restart";
    m_favoriteIdMapping["system-shutdown"] = "shutdown";

    m_favoritesModel = new PowerSessionFavoritesModel(group, this);

    connect(m_sessionModel, SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(countChanged()));
    connect(m_sessionModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(countChanged()));
    connect(m_sessionModel, SIGNAL(modelReset()), SIGNAL(countChanged()));

    connect(m_powerModel, SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(countChanged()));
    connect(m_powerModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(countChanged()));
    connect(m_powerModel, SIGNAL(modelReset()), SIGNAL(countChanged()));
}

CombinedPowerSessionModel::~CombinedPowerSessionModel()
{
}

int CombinedPowerSessionModel::count() const
{
    return rowCount();
}

int CombinedPowerSessionModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : (m_powerModel->count() + m_sessionModel->count());
}

QVariant CombinedPowerSessionModel::data(const QModelIndex &index, int role) const
{
    if (role == FavoriteIdRole) {
        QString iconName;

        if (index.row() >= m_sessionModel->count()) {
            iconName = m_powerModel->data(m_powerModel->index(index.row() - m_sessionModel->count(),
                index.column()), Qt::DecorationRole).toString();
        } else {
            iconName = m_sessionModel->data(m_sessionModel->index(index.row(), index.column()),
                Qt::DecorationRole).toString();
        }

        if (m_favoriteIdMapping.contains(iconName)) {
            return m_favoriteIdMapping[iconName];
        } else {
            return 0;
        }
    } else if (role == HasActionListRole) {
        return m_showFavoritesActions;
    } else if (role == ActionListRole) {
        QVariantList actionList;
        QVariantMap action;

        if (m_favoritesModel->isFavorite(index)) {
            action = ActionList::createActionItem(i18n("Remove from Sidebar"), "removeFromFavorites");
            action["icon"] = KIcon("list-remove");
            actionList << action;
        } else {
            action = ActionList::createActionItem(i18n("Add to Sidebar"), "addToFavorites");
            action["icon"] = KIcon("bookmark-new");
            actionList << action;
        }

        return actionList;
    }

    if (index.row() >= m_sessionModel->count()) {
        return m_powerModel->data(m_powerModel->index(index.row() - m_sessionModel->count(), index.column()), role);
    }

    return m_sessionModel->data(m_sessionModel->index(index.row(), index.column()), role);
}

bool CombinedPowerSessionModel::trigger(int row, const QString &actionId, const QVariant &)
{
    if (actionId == "addToFavorites") {
        QModelIndex idx = index(row);

        m_favoritesModel->addFavorite(idx);

        emit dataChanged(idx, idx);

        return false;
    } else if (actionId == "removeFromFavorites") {
        QModelIndex idx = index(row);

        m_favoritesModel->removeFavorite(idx);
        emit dataChanged(idx, idx);

        return false;
    }

    if (row >= m_sessionModel->count()) {
        return m_powerModel->trigger(row - m_sessionModel->count(), QString(), QVariant());
    }

    return m_sessionModel->trigger(row, QString(), QVariant());
}

bool CombinedPowerSessionModel::showFavoritesActions() const
{
    return m_showFavoritesActions;
}

void CombinedPowerSessionModel::setShowFavoritesActions(bool show)
{
    if (show != m_showFavoritesActions) {
        m_showFavoritesActions = show;

        emit dataChanged(index(0), index(count() - 1));
    }
}

QObject *CombinedPowerSessionModel::favoritesModel() const
{
    return m_favoritesModel;
}

QString CombinedPowerSessionModel::name() const
{
    return i18n("Power / Session");
}

CombinedPowerSessionSource::CombinedPowerSessionSource(QObject *parent) : AbstractSource(parent)
{
}

QAbstractItemModel *CombinedPowerSessionSource::createModelFromConfigGroup(const KConfigGroup &group)
{
    return new CombinedPowerSessionModel(group);
}

}

#include "combinedpowersessionmodel.moc"
