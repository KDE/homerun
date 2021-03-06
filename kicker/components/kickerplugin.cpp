/*
 *   Copyright 2013 Eike Hein <hein@kde.org>
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
#include <kickerplugin.h>

// Local
#include <appletproxy.h>
#include <fadeoutitem.h>
#include <processrunner.h>
#include <sourcelistmodel.h>
#include <urlconverter.h>
#include <windowsystem.h>

// Qt
#include <QtDeclarative/qdeclarative.h>

void KickerPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<AppletProxy>(uri, 0, 1, "AppletProxy");
    qmlRegisterType<FadeOutItem>(uri, 0, 1, "FadeOutItem");
    qmlRegisterType<ProcessRunner>(uri, 0, 1, "ProcessRunner");
    qmlRegisterType<SourceListModel>(uri, 0, 1, "SourceListModel");
    qmlRegisterType<UrlConverter>(uri, 0, 1, "UrlConverter");
    qmlRegisterType<WindowSystem>(uri, 0, 1, "WindowSystem");
}

#include "kickerplugin.moc"
