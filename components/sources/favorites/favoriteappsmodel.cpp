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
#include "favoriteappsmodel.h"

// Qt
#include <QDomDocument>
#include <QFile>

// KDE
#include <KDebug>
#include <KIcon>
#include <KConfigGroup>
#include <KLocale>
#include <KRun>
#include <KService>
#include <KStandardDirs>

namespace Homerun {

static QString localXmlFileName()
{
    return KStandardDirs::locateLocal("data", "homerun/favoriteapps.xml");
}

static QString systemXmlFileName()
{
    return KStandardDirs::locate("data", "homerun/favoriteapps.xml");
}

static QString serviceIdFromFavoriteId(const QString &favoriteId)
{
    if (!favoriteId.startsWith("app:")) {
        kWarning() << "Wrong favoriteId" << favoriteId;
        return QString();
    }
    return favoriteId.mid(4);
}

FavoriteAppsModel::FavoriteAppsModel(QObject *parent)
: QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::DecorationRole, "decoration");
    roles.insert(FavoriteIdRole, "favoriteId");
    setRoleNames(roles);
    load();
}

FavoriteAppsModel::~FavoriteAppsModel()
{
}

void FavoriteAppsModel::load()
{
    bool ok;
    QString name = localXmlFileName();
    if (QFile::exists(name)) {
        ok = loadFromXml(name);
        if (ok) {
            return;
        } else {
            kWarning() << "Failed to load from" << name;
        }
    }
    importFromConfigFile();
    if (m_favoriteList.isEmpty()) {
        // Nothing to import, load system xml file
        ok = loadFromXml(systemXmlFileName());
        if (!ok) {
            kWarning() << "Failed to load any favoriteapps file. No apps will be listed as favorite.";
        }
    }
}

bool FavoriteAppsModel::loadFromXml(const QString &fileName)
{
    QDomDocument doc;
    bool ok;
    {
        QFile file(fileName);
        ok = file.open(QIODevice::ReadOnly);
        if (!ok) {
            kWarning() << "Failed to open" << fileName << ". Error " << file.error();
            return false;
        }
        QString msg;
        int line, column;
        ok = doc.setContent(&file, &msg, &line, &column);
        if (!ok) {
            kWarning() << "Failed to parse" << fileName << ". Error line" << line << "column" << column << ":" << msg;
            return false;
        }
    }

    beginResetModel();
    m_favoriteList.clear();
    QDomElement root = doc.documentElement();
    for(QDomNode node = root.firstChild(); !node.isNull(); node = node.nextSibling()) {
        const QDomElement element = node.toElement();
        if (element.isNull()) {
            kWarning() << "Ignoring invalid node";
            continue;
        }
        const QString serviceId = element.attribute("serviceId");
        if (serviceId.isEmpty()) {
            kWarning() << "Ignoring element with empty serviceId attribute";
            continue;
        }
        KService::Ptr service = KService::serviceByStorageId(serviceId);
        if (service.isNull()) {
            kWarning() << "Ignoring element with invalid serviceId" << serviceId;
            continue;
        }
        FavoriteInfo info = { service };
        m_favoriteList << info;
    }
    endResetModel();
    countChanged();

    return true;
}

void FavoriteAppsModel::saveToXml()
{
    bool ok;

    QDomDocument doc;
    QDomElement root = doc.createElement("apps");
    root.setAttribute("version", "1");
    doc.appendChild(root);

    Q_FOREACH(const FavoriteInfo &info, m_favoriteList) {
        QString serviceId = info.service->storageId();
        QDomElement element = doc.createElement("app");
        element.setAttribute("serviceId", serviceId);
        root.appendChild(element);
    }

    QFile file(localXmlFileName());
    ok = file.open(QIODevice::WriteOnly);
    if (!ok) {
        kWarning() << "Failed to open" << localXmlFileName() << "for writing. Error" << file.error();
        return;
    }
    file.write(doc.toByteArray(4));
}

void FavoriteAppsModel::addFavorite(const QString &favoriteId)
{
    QString serviceId = serviceIdFromFavoriteId(favoriteId);
    if (serviceId.isEmpty()) {
        return;
    }
    KService::Ptr service = KService::serviceByStorageId(serviceId);
    if (service.isNull()) {
        kWarning() << "Could not find a service for" << serviceId;
        return;
    }
    FavoriteInfo info = { service };

    int row = m_favoriteList.count();
    beginInsertRows(QModelIndex(), row, row);
    m_favoriteList << info;
    endInsertRows();
    countChanged();

    saveToXml();
}

void FavoriteAppsModel::removeFavorite(const QString &favoriteId)
{
    int row = rowForFavoriteId(favoriteId);
    if (row == -1) {
        kWarning() << "Could not find favorite" << favoriteId;
        return;
    }
    beginRemoveRows(QModelIndex(), row, row);
    FavoriteInfo info = m_favoriteList.takeAt(row);
    endRemoveRows();
    countChanged();

    saveToXml();
}

bool FavoriteAppsModel::isFavorite(const QString &favoriteId) const
{
    return rowForFavoriteId(favoriteId) != -1;
}

int FavoriteAppsModel::rowForFavoriteId(const QString& favoriteId) const
{
    QString serviceId = serviceIdFromFavoriteId(favoriteId);
    if (serviceId.isEmpty()) {
        return -1;
    }

    for (int row = m_favoriteList.count() - 1; row >= 0; --row) {
        const FavoriteInfo& info = m_favoriteList.at(row);
        if (info.service->storageId() == serviceId) {
            return row;
        }
    }
    return -1;
}

int FavoriteAppsModel::count() const
{
    return m_favoriteList.count();
}

QString FavoriteAppsModel::name() const
{
    return i18n("Favorite Applications");
}

QString FavoriteAppsModel::favoritePrefix() const
{
    return "app";
}

bool FavoriteAppsModel::canMoveRow() const
{
    return true;
}

int FavoriteAppsModel::rowCount(const QModelIndex &index) const
{
    if (index.isValid()) {
        return 0;
    }
    return m_favoriteList.count();
}

QVariant FavoriteAppsModel::data(const QModelIndex &index, int role) const
{
    KService::Ptr service = m_favoriteList.value(index.row()).service;
    if (service.isNull()) {
        return QVariant();
    }
    if (role == Qt::DisplayRole) {
        return service->name();
    } else if (role == Qt::DecorationRole) {
        return service->icon();
    } else if (role == FavoriteIdRole) {
        return QVariant("app:" + service->storageId());
    } else {
        kWarning() << "Unhandled role" << role;
        return QVariant();
    }
}

bool FavoriteAppsModel::trigger(int row)
{
    KService::Ptr service = m_favoriteList.value(row).service;
    if (service.isNull()) {
        kWarning() << "Invalid row";
        return false;
    }
    return KRun::run(*service, KUrl::List(), 0);
}

#define CHECK_ROW(row) \
    if (row < 0 || row >= m_favoriteList.count()) { \
        kWarning() << "Invalid row number" << row; \
        return; \
    }

void FavoriteAppsModel::moveRow(int from, int to)
{
    CHECK_ROW(from)
    CHECK_ROW(to)
    if (from == to) {
        kWarning() << "Cannot move row to itself";
        return;
    }
    // See beginMoveRows() doc for an explanation on modelTo
    int modelTo = to + (to > from ? 1 : 0);
    bool ok = beginMoveRows(QModelIndex(), from, from, QModelIndex(), modelTo);
    if (!ok) {
        kWarning() << "beginMoveRows failed";
        Q_ASSERT(!"beginMoveRows failed");
    }
    m_favoriteList.move(from, to);
    endMoveRows();

    saveToXml();
}

void FavoriteAppsModel::importFromConfigFile()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig("homerunrc", KConfig::SimpleConfig);
    KConfigGroup baseGroup(config, "favorites");
    if (!baseGroup.exists()) {
        // No favorite to import. Leave now, we do not want to write an empty
        // xml file
        return;
    }

    // Get favorites in a map to order them correctly
    QMap<int, KService::Ptr> favoriteMap;
    Q_FOREACH(const QString &favoriteGroup, baseGroup.groupList()) {
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

    // Load favorites following map order
    beginResetModel();
    auto it = favoriteMap.constBegin(), end = favoriteMap.constEnd();
    for (; it != end; ++it) {
        FavoriteInfo info = { it.value() };
        m_favoriteList << info;
    }

    // Finish migration
    saveToXml();
    config->deleteGroup("favorites");
    config->sync();

    // Notify outside
    endResetModel();
    countChanged();
}

} // namespace Homerun

#include "favoriteappsmodel.moc"
