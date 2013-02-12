/*
Copyright 2013 Aurélien Gâteau <agateau@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
// Self
#include <querymatchmodel.h>

// KDE
#include <KDebug>
#include <KLocale>
#include <KUrl>
#include <Plasma/AbstractRunner>

// Qt
#include <QIcon>

namespace Homerun
{

QueryMatchModel::QueryMatchModel(QObject *parent)
: QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::DecorationRole, "decoration");
    roles.insert(FavoriteIdRole, "favoriteId");
    roles.insert(HasActionListRole, "hasActionList");
    roles.insert(ActionListRole, "actionList");
    setRoleNames(roles);
}

int QueryMatchModel::count() const
{
    return m_matches.count();
}

int QueryMatchModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_matches.count();
}

QVariant QueryMatchModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= m_matches.count()) {
        return QVariant();
    }
    Plasma::QueryMatch match = m_matches.at(index.row());
    if (role == Qt::DisplayRole) {
        return match.text();
    } else if (role == Qt::DecorationRole) {
        return match.icon();
    } else if (role == FavoriteIdRole) {
        QString runnerId = match.runner()->id();
        if (runnerId == "services") {
            return QVariant("app:" + match.data().toString());
        } else if (runnerId == "locations") {
            KUrl url(match.data().toString());
            return QVariant("place:" + url.url());
        } else if (runnerId == "places") {
            KUrl url(match.data().value<KUrl>());
            return QVariant("place:" + url.url());
        } else {
            return QString();
        }
    } else if (role == HasActionListRole) {
        return false;
    }
    return QVariant();
}

void QueryMatchModel::setMatches(const QList< Plasma::QueryMatch > &matches)
{
    bool fullReset = false;
    int oldCount = m_matches.count();
    int newCount = matches.count();
    if (newCount > oldCount) {
        // We received more matches than we had. If all common matches are the
        // same, we can just append new matches instead of resetting the whole
        // model
        for (int row = 0; row < oldCount; ++row) {
            if (!(m_matches.at(row) == matches.at(row))) {
                fullReset = true;
                break;
            }
        }
        if (!fullReset) {
            // Not a full reset, inserting rows
            beginInsertRows(QModelIndex(), oldCount, newCount);
            m_matches = matches;
            endInsertRows();
            emit countChanged();
        }
    } else {
        fullReset = true;
    }

    if (fullReset) {
        beginResetModel();
        m_matches = matches;
        endResetModel();
        emit countChanged();
    }
}

} // namespace

#include <querymatchmodel.moc>
