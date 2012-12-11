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

// Qt
#include <QGraphicsLinearLayout>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>

// KDE
#include <KRun>
#include <KStandardDirs>
#include <KShortcut>
#include <Plasma/IconWidget>
#include <Plasma/Containment>

// Local
#include <configkeys.h>
#include <configmanager.h>
#include <homerunlauncher.h>

HomerunLauncher::HomerunLauncher(QObject * parent, const QVariantList & args)
    : Plasma::Applet(parent, args),
      m_icon(0)
{
    setHasConfigurationInterface(true);
    KGlobal::locale()->insertCatalog("plasma_applet_org.kde.homerun");
}

void HomerunLauncher::init()
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_icon = new Plasma::IconWidget(this);

    connect(m_icon, SIGNAL(clicked()), SLOT(toggle()));
    connect(this, SIGNAL(activate()), SLOT(toggle()));

    layout->addItem(m_icon);

    readConfig();

    if (!isViewerRunning()) {
        kDebug() << "Service not registered, launching homerunviewer";
        startViewer(HomerunLauncher::DontShow);
    }
}

bool HomerunLauncher::isViewerRunning() const
{
    return QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.homerunViewer");
}

void HomerunLauncher::startViewer(int screen)
{
    QString cmd = "homerunviewer";
    if (screen != DontShow) {
        cmd += " --show " + QString::number(screen);
    }
    KRun::runCommand(cmd, 0);
}

void HomerunLauncher::toggle()
{
    int screen = containment()->screen();
    if (!isViewerRunning()) {
        startViewer(screen);
        return;
    }

    QDBusConnection bus = QDBusConnection::sessionBus();
    QDBusInterface interface("org.kde.homerunViewer", "/HomerunViewer", "org.kde.homerunViewer", bus);
    interface.asyncCall("toggle", screen);
}

void HomerunLauncher::createConfigurationInterface(KConfigDialog *dialog)
{
    ConfigManager *manager = new ConfigManager(config(), dialog);
    connect(manager, SIGNAL(configChanged()), SLOT(readConfig()));
}

void HomerunLauncher::readConfig()
{
    m_icon->setIcon(config().readEntry(CFG_LAUNCHER_ICON_KEY, CFG_LAUNCHER_ICON_DEFAULT));
}

#include "homerunlauncher.moc"
