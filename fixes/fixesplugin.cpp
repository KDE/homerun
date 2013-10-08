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

// Self
#include <fixesplugin.h>

// Local
#include <datamodel.h>
<<<<<<< HEAD
#include <toolboxtoggle.h>
=======
#include <sourcelistmodel.h>
>>>>>>> Proper search, proper source reload, KRunner support.
#include <urlconverter.h>

// Qt
#include <QtDeclarative/qdeclarative.h>

void FixesPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<Plasma::SortFilterModel>(uri, 0, 1, "SortFilterModel");
    qmlRegisterType<ToolBoxToggle>(uri, 0, 1, "ToolBoxToggle");
    qmlRegisterType<SourceListModel>(uri, 0, 1, "SourceListModel");
    qmlRegisterType<SourceListFilterModel>(uri, 0, 1, "SourceListFilterModel");
    qmlRegisterType<UrlConverter>(uri, 0, 1, "UrlConverter");
}

#include "fixesplugin.moc"
