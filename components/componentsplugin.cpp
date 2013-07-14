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

#include <action.h>
#include <actionmanager.h>
#include <globalsettings.h>
#include <helpmenuactions.h>
#include <icondialog.h>
#include <image.h>
#include <messagebox.h>
#include <shadoweffect.h>
#include <sourceregistry.h>
#include <tabmodel.h>

void ComponentsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.homerun.components"));
    qmlRegisterType<Action>(uri, 0, 1, "Action");
    qmlRegisterType<ActionManager>(uri, 0, 1, "ActionManager");
    qmlRegisterType<ShadowEffect>(uri, 0, 1, "ShadowEffect");
    qmlRegisterType<GlobalSettings>(uri, 0, 1, "GlobalSettings");
    qmlRegisterType<HelpMenuActions>(uri, 0, 1, "HelpMenuActions");
    qmlRegisterType<IconDialog>(uri, 0, 1, "IconDialog");
    qmlRegisterType<Image>(uri, 0, 1, "Image");
    qmlRegisterType<MessageBox>(uri, 0, 1, "MessageBox");
    qmlRegisterType<Homerun::AbstractSourceRegistry>(uri, 0, 1, "AbstractSourceRegistry");
    qmlRegisterType<Homerun::SourceRegistry>(uri, 0, 1, "SourceRegistry");
    qmlRegisterType<TabModel>(uri, 0, 1, "TabModel");
}

#include "componentsplugin.moc"
