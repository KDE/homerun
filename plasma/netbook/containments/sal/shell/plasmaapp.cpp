/*
 *   Copyright 2006 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 Chani Armitage <chanika@gmail.com>
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "plasmaapp.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QPixmapCache>
#include <QtDBus/QtDBus>

#include <KDebug>
#include <KCmdLineArgs>
#include <KWindowSystem>


#include <Plasma/Containment>
#include <Plasma/Theme>
#include <Plasma/Dialog>

#include "salcorona.h"
#include "salview.h"

PlasmaApp::PlasmaApp()
    : KUniqueApplication(),
      m_corona(0)
{
    //load translations for libplasma
    KGlobal::locale()->insertCatalog("libplasma");
    KGlobal::locale()->insertCatalog("plasmagenericshell");

    // this line initializes the corona.
    corona();
}

PlasmaApp::~PlasmaApp()
{
}

//void PlasmaApp::createWaitingViews()
//{
//           SalView *view = viewForScreen(containment->screen());
//            view = new SalView(containment, 0);
//}

SalCorona* PlasmaApp::corona()
{
    if (!m_corona) {
        m_corona = new SalCorona(this);
        connect(m_corona, SIGNAL(screenOwnerChanged(int,int,Plasma::Containment*)),
                this, SLOT(containmentScreenOwnerChanged(int,int,Plasma::Containment*)));
        connect(m_corona, SIGNAL(configSynced()), SLOT(syncConfig()));
        //kDebug() << "connected to containmentAdded";
        /*
        foreach (DesktopView *view, m_desktops) {
            connect(c, SIGNAL(screenOwnerChanged(int,int,Plasma::Containment*)),
                            view, SLOT(screenOwnerChanged(int,int,Plasma::Containment*)));
        }*/

        m_corona->setItemIndexMethod(QGraphicsScene::NoIndex);
        m_corona->initializeLayout();

        //we want this *after* init so that we ignore any lock/unlock spasms that might happen then
        connect(m_corona, SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)), this, SLOT(immutabilityChanged(Plasma::ImmutabilityType)));

        //kDebug() << "layout should exist";
        //c->checkScreens();
    }

    return m_corona;
}


#include "plasmaapp.moc"
