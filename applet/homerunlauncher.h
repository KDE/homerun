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

#ifndef HOMERUNLAUNCHER_H
#define HOMERUNLAUNCHER_H

#include <QtCore/QBasicTimer>
#include <QtCore/QTime>
#include <QtGui/QVector2D>
#include <KConfigDialog>

#include <Plasma/Applet>
#include <Plasma/Svg>

#include <Phonon/MediaObject>
#include <Phonon/Path>
#include <Phonon/AudioOutput>
#include <Phonon/Global>

namespace Plasma {
    class IconWidget;
}

class QGraphicsSceneMouseEvent;
class QSizeF;

class HomerunLauncher : public Plasma::Applet
{
    Q_OBJECT
    enum {
        DontShow = -2
    };
public:
    HomerunLauncher(QObject * parent, const QVariantList & args);

    void init();

private Q_SLOTS:
    void toggle();

private:
    Plasma::IconWidget *m_icon;

    void startViewer(int screen);
    bool isViewerRunning() const;
};

K_EXPORT_PLASMA_APPLET (homerunlauncher, HomerunLauncher)
#endif
