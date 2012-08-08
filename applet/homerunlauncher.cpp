/***************************************************************************
 *   Copyright (C) 2012 by Shaun Reich <shaun.reich@blue-systems.com>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "homerunlauncher.h"
#include <QtCore/QSizeF>
#include <QtGui/QPainter>
#include <QtGui/QDesktopWidget>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsLinearLayout>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>

#include <Plasma/PushButton>
#include <Plasma/IconWidget>
#include <Plasma/Containment>

#include <KSharedConfig>
#include <KLocale>
#include <KStandardDirs>
#include <KIO/NetAccess>
#include <KMessageBox>
#include <KToolInvocation>
#include <KRun>

HomerunLauncher::HomerunLauncher(QObject * parent, const QVariantList & args)
    : Plasma::Applet(parent, args),
      m_icon(0)
{
    setHasConfigurationInterface(true);
    KGlobal::locale()->insertCatalog("plasma_applet_org.kde.homerun");
//    resize(contentSizeHint());
}

void HomerunLauncher::init()
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_icon = new Plasma::IconWidget(this);
    m_icon->setIcon("homerun");

    connect(m_icon, SIGNAL(clicked()), SLOT(toggle()));
    connect(this, SIGNAL(activate()), SLOT(toggle()));

    layout->addItem(m_icon);

    //oddly doesn't work?
    //KToolInvocation::startServiceByDesktopPath("homerunviewer.desktop", QStringList(), &error);
    //kDebug() << "ERROR?: " << error;
    checkAndLaunch();
}

void HomerunLauncher::checkAndLaunch()
{
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.homerunViewer")) {
        kDebug() << "Service not registered, launching homerunviewer";
        KRun::runCommand("homerunviewer", 0);
    }
}

void HomerunLauncher::toggle()
{
    kDebug() << "ICON CLICKED!";
    checkAndLaunch();

    QDBusConnection bus = QDBusConnection::sessionBus();

    QDBusInterface interface("org.kde.homerunViewer", "/HomerunViewer", "org.kde.homerunViewer", bus);

    interface.asyncCall("toggle", containment()->screen());
}

#include "homerunlauncher.moc"
