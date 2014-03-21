/***************************************************************************
 *   Copyright (C) 2012 by Shaun Reich <shaun.reich@blue-systems.com>      *
 *   Copyright (C) 2013 by Eike Hein <hein@kde.org>                        *
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

#ifndef HOMERUNLAUNCHER_H
#define HOMERUNLAUNCHER_H

// KDE
#include <Plasma/Applet>

// Local
#include <ui_generalconfigpage.h>

namespace Plasma {
    class IconWidget;
}

class QDBusServiceWatcher;
class QGraphicsSceneMouseEvent;
class QSizeF;

class HomerunLauncher : public Plasma::Applet
{
    Q_OBJECT
public:
    HomerunLauncher(QObject * parent, const QVariantList & args);

    void init();

    void createConfigurationInterface(KConfigDialog *parent); // reimp

    QList<QAction *> contextualActions();

    public slots:
        void configChanged();

private Q_SLOTS:
    void toggle();
    void configAccepted();
    void startMenuEditor();
    void viewerServiceRegistered();
    void viewerServiceUnregistered();
    void addToDesktop(uint containmentId, const QString &storageId);
    void addToPanel(uint containmentId, const QString &storageId);

private:
    Plasma::IconWidget *m_icon;
    QDBusServiceWatcher *m_serviceWatcher;
    bool m_serviceRegistered;
    bool m_toggleWhenRegistered;
    QList<QAction *> actions;
    Ui_GeneralConfigPage m_generalUi;

    void startViewer();
};

K_EXPORT_PLASMA_APPLET (homerunlauncher, HomerunLauncher)
#endif
