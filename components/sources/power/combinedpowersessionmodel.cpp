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

    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(countChanged()));
    connect(this, SIGNAL(modelReset()), SIGNAL(countChanged()));

    m_favorites = group.readEntry("Favorites", QList<int>() << 3 << 5 << 6);
}

PowerSessionFavoritesModel::~PowerSessionFavoritesModel()
{
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

    return m_combinedPowerSessionModel->data(m_combinedPowerSessionModel->index(m_favorites.at(index.row())), role);

    return QVariant();
}

bool PowerSessionFavoritesModel::trigger(int row, const QString &actionId, const QVariant &)
{
    if (row > m_favorites.count() - 1) {
        return false;
    }

    return m_combinedPowerSessionModel->trigger(m_favorites.at(row), actionId, QString());
}

void PowerSessionFavoritesModel::moveRow(int from, int to)
{
    beginMoveRows(QModelIndex(), from, from, QModelIndex(), to > from ? to + 1 : to);

    m_favorites.move(from, to);

    endMoveRows();

    m_configGroup.writeEntry("Favorites", m_favorites);
    m_configGroup.config()->sync();
}

void PowerSessionFavoritesModel::clear()
{
    beginResetModel();

    m_favorites.clear();

    endResetModel();
}

void PowerSessionFavoritesModel::addFavorite(int row)
{
    beginInsertRows(QModelIndex(), m_favorites.count(), m_favorites.count());

    m_favorites.append(row);

    endInsertRows();

    m_configGroup.writeEntry("Favorites", m_favorites);
    m_configGroup.config()->sync();
}

void PowerSessionFavoritesModel::removeFavorite(int row)
{
    int index = m_favorites.indexOf(row);

    if (index != -1) {
        beginRemoveRows(QModelIndex(), index, index);

        m_favorites.removeAt(index);

        endRemoveRows();

        m_configGroup.writeEntry("Favorites", m_favorites);
        m_configGroup.config()->sync();
    }
}

bool PowerSessionFavoritesModel::isFavorite(int row)
{
    return m_favorites.contains(row);
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
    int row = index.row();

    if (role == HasActionListRole) {
        return m_showFavoritesActions;
    } else if (role == ActionListRole) {
        QVariantList actionList;
        QVariantMap action;

        if (m_favoritesModel->isFavorite(row)) {
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

    if (row >= m_sessionModel->count()) {
        return m_powerModel->data(m_powerModel->index(row - m_sessionModel->count(), index.column()), role);
    }

    return m_sessionModel->data(m_sessionModel->index(index.row(), index.column()), role);
}

bool CombinedPowerSessionModel::trigger(int row, const QString &actionId, const QVariant &)
{
    if (actionId == "addToFavorites") {
        m_favoritesModel->addFavorite(row);

        QModelIndex idx = index(row);
        emit dataChanged(idx, idx);

        return false;
    } else if (actionId == "removeFromFavorites") {
        m_favoritesModel->removeFavorite(row);

        QModelIndex idx = index(row);
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
