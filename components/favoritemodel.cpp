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
}

FavoriteModel::~FavoriteModel()
{
}

QString FavoriteModel::configFileName() const
{
    return m_config.isNull() ? QString() : m_config->name();
}

void FavoriteModel::setConfigFileName(const QString &name)
{
    if (name == configFileName()) {
        return;
    }
    setConfig(KSharedConfig::openConfig(name));
}

void FavoriteModel::setConfig(const KSharedConfig::Ptr &ptr)
{
    m_config = ptr;

    KConfigGroup baseGroup(m_config, "favorites");

    // get all the favorites
    QMap<uint, KConfigGroup> favoritesConfigs;
    foreach (const QString &favoriteGroup, baseGroup.groupList()) {
        if (favoriteGroup.startsWith("favorite-")) {
            KConfigGroup favoriteConfig(&baseGroup, favoriteGroup);
            favoritesConfigs.insert(favoriteGroup.split("-").last().toUInt(), favoriteConfig);
        }
    }

    QStringList names;
    Q_FOREACH(const KConfigGroup &config, favoritesConfigs) {
        names << config.readEntry("serviceId");
    }

    beginResetModel();
    m_favoriteList.clear();
    Q_FOREACH(const QString &name, names) {
        KService::Ptr service = KService::serviceByStorageId(name);
        if (!service.isNull()) {
            m_favoriteList << service;
        }
    }
    endResetModel();
    countChanged();
    configFileNameChanged();
}

void FavoriteModel::append(const QString &serviceId)
{
    KService::Ptr service = KService::serviceByStorageId(serviceId);
    if (service.isNull()) {
        kWarning() << "Could not find a service for" << serviceId;
        return;
    }
    int row = m_favoriteList.count();
    beginInsertRows(QModelIndex(), row, row);
    m_favoriteList << service;
    endInsertRows();
    countChanged();

    KConfigGroup baseGroup(m_config, "favorites");
    KConfigGroup group(&baseGroup, QString("favorite-%1").arg(row));
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
    m_favoriteList.removeAt(row);
    endRemoveRows();
    countChanged();

    KConfigGroup baseGroup(m_config, "favorites");
    KConfigGroup group(&baseGroup, QString("favorite-%1").arg(row));
    group.deleteGroup();
    baseGroup.sync();
}

int FavoriteModel::count() const
{
    return m_favoriteList.count();
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
    KService::Ptr service = m_favoriteList.value(index.row());
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

void FavoriteModel::run(int row)
{
    KService::Ptr service = m_favoriteList.value(row);
    if (service.isNull()) {
        kWarning() << "Invalid row";
        return;
    }
    KRun::run(*service, KUrl::List(), 0);
}

#include "favoritemodel.moc"
