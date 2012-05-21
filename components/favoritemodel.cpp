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

static KService::Ptr findService(const QString &name)
{
    KService::Ptr service = KService::serviceByDesktopPath(name);
    if (!service) {
        service = KService::serviceByDesktopName(name);
    }
    return service;
}

FavoriteModel::FavoriteModel(QObject *parent)
: QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "label");
    roles.insert(Qt::DecorationRole, "icon");
    roles.insert(EntryPathRole, "entryPath");

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
        names << config.readEntry("url");
    }

    beginResetModel();
    m_favoriteList.clear();
    Q_FOREACH(const QString &name, names) {
        KService::Ptr service = findService(name);
        if (!service.isNull()) {
            m_favoriteList << service;
        }
    }
    endResetModel();
    countChanged();
    configFileNameChanged();
}

void FavoriteModel::add(const QString &entryPath)
{
    KService::Ptr service = findService(entryPath);
    if (service.isNull()) {
        kWarning() << "Could not find a service for" << entryPath;
        return;
    }
    int row = m_favoriteList.count();
    beginInsertRows(QModelIndex(), row, row);
    m_favoriteList << service;
    endInsertRows();
    countChanged();

    KConfigGroup baseGroup(m_config, "favorites");
    KConfigGroup group(&baseGroup, QString("favorite-%1").arg(row));
    group.writeEntry("url", entryPath);
    baseGroup.sync();
}

void FavoriteModel::remove(const QString &entryPath)
{
    int row = indexOfByPath(entryPath);
    if (row == -1) {
        kWarning() << entryPath << "is not in the favorites";
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

bool FavoriteModel::isFavorite(const QString &entryPath) const
{
    return indexOfByPath(entryPath) != -1;
}

int FavoriteModel::indexOfByPath(const QString &entryPath) const
{
    int row;
    for (row = m_favoriteList.count() - 1; row >= 0; --row) {
        if (m_favoriteList[row]->entryPath() == entryPath) {
            break;
        }
    }
    return row;
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
    } else if (role == EntryPathRole) {
        return service->entryPath();
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

//void FavoriteModel::add(const QUrl &url, const QModelIndex &before)
//{
//
//    KService::Ptr service = KService::serviceByDesktopPath(url.path());
//
//    if (!service) {
//        service = KService::serviceByDesktopName(url.path());
//    }
//
//    if (!service) {
//        if (!url.isValid()) {
//            return;
//        }
//
//        QString query = url.path();
//        QString runnerId = url.host();
//        QString matchId = url.fragment();
//        if (matchId.startsWith(QLatin1Char('/'))) {
//            matchId = matchId.remove(0, 1);
//        }
//
//        //FIXME: another inefficient async query
//        runnerManager()->blockSignals(true);
//        runnerManager()->execQuery(query, runnerId);
//        runnerManager()->blockSignals(false);
//
//        Plasma::QueryMatch match(runnerManager()->searchContext()->match(matchId));
//
//        if (match.isValid()) {
//            if (before.isValid()) {
//                insertRow(
//                    before.row(),
//                    StandardItemFactory::createItem(
//                        match.icon(),
//                        match.text(),
//                        match.subtext(),
//                        url.path(),
//                        1, //don't need weigt here
//                        CommonModel::RemoveAction
//                        )
//                    );
//            } else {
//                appendRow(
//                    StandardItemFactory::createItem(
//                        match.icon(),
//                        match.text(),
//                        match.subtext(),
//                        url.path(),
//                        1, //don't need weigt here
//                        CommonModel::RemoveAction
//                        )
//                    );
//            }
//        }
//    } else {
//        if (before.isValid()) {
//            insertRow(
//                before.row(),
//                StandardItemFactory::createItem(
//                    KIcon(service->icon()),
//                    service->name(),
//                    service->genericName(),
//                    service->entryPath(),
//                    1, //don't need weigt here
//                    CommonModel::RemoveAction
//                    )
//                );
//        } else {
//            appendRow(
//                StandardItemFactory::createItem(
//                    KIcon(service->icon()),
//                    service->name(),
//                    service->genericName(),
//                    service->entryPath(),
//                    1, //don't need weigt here
//                    CommonModel::RemoveAction
//                    )
//                );
//        }
//    }
//}
//
//void FavoriteModel::save(KConfigGroup &cg)
//{
//    kDebug() << "----------------> Saving Stuff...";
//
//    // erase the old stuff before saving the new one
//    KConfigGroup oldGroup(&cg, "stripwidget");
//    oldGroup.deleteGroup();
//
//    KConfigGroup stripGroup(&cg, "stripwidget");
//
//    for (int i = 0; i <= rowCount(); i++) {
//        QModelIndex currentIndex = index(i, 0);
//        KConfigGroup config(&stripGroup, QString("favorite-%1").arg(i));
//        QString url = currentIndex.data(CommonModel::Url).value<QString>();
//        if (!url.isNull()) {
//            config.writeEntry("url", url);
//        }
//    }
//}

#include "favoritemodel.moc"
