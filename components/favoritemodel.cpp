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

// Own
#include "favoritemodel.h"

// Qt

// KDE
#include <KDebug>
#include <KIcon>
#include <KLocale>
#include <KRun>
#include <KService>

//Plasma
#include <Plasma/AbstractRunner>
#include <Plasma/RunnerManager>

FavoriteModel::FavoriteModel(QObject *parent)
: QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "label");
    roles.insert(Qt::DecorationRole, "icon");
    setRoleNames(roles);
    setConfig(KSharedConfig::openConfig("salrc"));
}

FavoriteModel::~FavoriteModel()
{
}

void FavoriteModel::setConfig(const KSharedConfig::Ptr &ptr)
{
    m_config = ptr;

    KConfigGroup baseGroup(m_config, "favorites");

    // get all the favorites
    QMap<int, KService::Ptr> favoriteMap;
    foreach (const QString &favoriteGroup, baseGroup.groupList()) {
        if (favoriteGroup.startsWith("favorite-")) {
            KConfigGroup favoriteConfig(&baseGroup, favoriteGroup);
            int rank = favoriteGroup.split("-").last().toInt();
            QString id = favoriteConfig.readEntry("serviceId");
            KService::Ptr service = KService::serviceByStorageId(id);
            if (!service.isNull()) {
                favoriteMap.insert(rank, service);
            }
        }
    }

    beginResetModel();
    m_favoriteList.clear();
    auto it = favoriteMap.constBegin(), end = favoriteMap.constEnd();
    for (; it != end; ++it) {
        FavoriteInfo info = { it.key(), it.value() };
        m_favoriteList << info;
    }
    endResetModel();
    countChanged();
}

void FavoriteModel::append(const QString &serviceId)
{
    KService::Ptr service = KService::serviceByStorageId(serviceId);
    if (service.isNull()) {
        kWarning() << "Could not find a service for" << serviceId;
        return;
    }
    int rank;
    if (!m_favoriteList.isEmpty()) {
        rank = m_favoriteList.last().rank + 1;
    } else {
        rank = 0;
    }
    FavoriteInfo info = { rank, service };

    int row = m_favoriteList.count();
    beginInsertRows(QModelIndex(), row, row);
    m_favoriteList << info;
    endInsertRows();
    countChanged();

    KConfigGroup baseGroup(m_config, "favorites");
    KConfigGroup group(&baseGroup, QString("favorite-%1").arg(rank));
    group.writeEntry("serviceId", serviceId);
    baseGroup.sync();
}

void FavoriteModel::removeAt(int row)
{
    if (row < 0 || row >= m_favoriteList.count()) {
        kWarning() << "Invalid row" << row;
        return;
    }
    beginRemoveRows(QModelIndex(), row, row);
    FavoriteInfo info = m_favoriteList.takeAt(row);
    endRemoveRows();
    countChanged();

    KConfigGroup baseGroup(m_config, "favorites");
    KConfigGroup group(&baseGroup, QString("favorite-%1").arg(info.rank));
    group.deleteGroup();
    baseGroup.sync();
}

int FavoriteModel::count() const
{
    return m_favoriteList.count();
}

QString FavoriteModel::name() const
{
    return i18n("Favorite Applications");
}

int FavoriteModel::rowCount(const QModelIndex &index) const
{
    if (index.isValid()) {
        return 0;
    }
    return m_favoriteList.count();
}

QVariant FavoriteModel::data(const QModelIndex &index, int role) const
{
    KService::Ptr service = m_favoriteList.value(index.row()).service;
    if (service.isNull()) {
        return QVariant();
    }
    if (role == Qt::DisplayRole) {
        return service->name();
    } else if (role == Qt::DecorationRole) {
        return KIcon(service->icon());
    } else {
        kWarning() << "Unhandled role" << role;
        return QVariant();
    }
}

bool FavoriteModel::trigger(int row)
{
    KService::Ptr service = m_favoriteList.value(row).service;
    if (service.isNull()) {
        kWarning() << "Invalid row";
        return false;
    }
    return KRun::run(*service, KUrl::List(), 0);
}

#include "favoritemodel.moc"
