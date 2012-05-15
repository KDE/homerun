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

#include "sallauncher.h"
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

SalLauncher::SalLauncher(QObject * parent, const QVariantList & args)
    : Plasma::Applet(parent, args),
      m_icon(0)
{
    setHasConfigurationInterface(true);
//    resize(contentSizeHint());
}

void SalLauncher::init()
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);

    m_icon = new Plasma::IconWidget(this);
    m_icon->setIcon("kde");

    connect(m_icon, SIGNAL(clicked()), SLOT(toggleSalViewer()));
    connect(this, SIGNAL(activate()), SLOT(toggleSalViewer()));

    layout->addItem(m_icon);

    //oddly doesn't work?
    //KToolInvocation::startServiceByDesktopPath("salviewer.desktop", QStringList(), &error);
    //kDebug() << "ERROR?: " << error;
    checkAndLaunch();
}

void SalLauncher::checkAndLaunch()
{
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.salViewer")) {
        kDebug() << "Service not registered, launching salviewer";
        KRun::runCommand("salviewer", 0);
    }
}

void SalLauncher::toggleSalViewer()
{
    kDebug() << "ICON CLICKED!";
    checkAndLaunch();

    QDBusConnection bus = QDBusConnection::sessionBus();

    QDBusInterface interface("org.kde.salViewer", "/SalViewer", "org.kde.salViewer", bus);

    interface.asyncCall("toggle", containment()->screen());
}

#include "sallauncher.moc"
