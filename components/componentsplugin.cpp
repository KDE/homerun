/*
 *   Copyright 2012 Shaun Reich <shaun.reich@blue-systems.com>
 *   Copyright 2012 by Aurélien Gâteau <agateau@kde.org>
 *
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

#include "componentsplugin.h"

#include <QtDeclarative/qdeclarative.h>

#include <Plasma/QueryMatch>

#include "sharedconfig.h"
#include "favoriteappsmodel.h"
#include "groupedservicemodel.h"
#include "pagemodel.h"
#include "placesmodel.h"
#include "powermodel.h"
#include "runnerinfoprovider.h"
#include "runnermodel.h"
#include "servicemodel.h"
#include "sessionmodel.h"
#include "sourcepluginloader.h"

void ComponentsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.homerun.components"));
    qmlRegisterType<FavoriteAppsModel>(uri, 0, 1, "FavoriteAppsModel");
    qmlRegisterType<FavoritePlacesModel>(uri, 0, 1, "FavoritePlacesModel");
    qmlRegisterType<GroupedServiceModel>(uri, 0, 1, "GroupedServiceModel");
    qmlRegisterType<PageModel>(uri, 0, 1, "PageModel");
    qmlRegisterType<PlacesModel>(uri, 0, 1, "PlacesModel");
    qmlRegisterType<PowerModel>(uri, 0, 1, "PowerModel");
    qmlRegisterType<SessionModel>(uri, 0, 1, "SessionModel");
    qmlRegisterType<RunnerInfoProvider>(uri, 0, 1, "RunnerInfoProvider");
    qmlRegisterType<SharedConfig>(uri, 0, 1, "SharedConfig");
    qmlRegisterType<RunnerModel>(uri, 0, 1, "RunnerModel");
    qmlRegisterType<ServiceModel>(uri, 0, 1, "ServiceModel");
    qmlRegisterType<SourcePluginLoader>(uri, 0, 1, "SourcePluginLoader");
    qmlRegisterInterface<Plasma::QueryMatch>("QueryMatch");
    qRegisterMetaType<Plasma::QueryMatch *>("QueryMatch");
}

#include "componentsplugin.moc"

