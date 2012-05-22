/*
 *   Copyright 2012 Aurélien Gâteau <agateau@kde.org>
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

import Qt 4.7
import org.kde.qtextracomponents 0.1 as QtExtra

QtExtra.QIconItem {
    id: main
    anchors.centerIn: parent
    icon: "bookmarks"
    width: 128
    height: width
    scale: 32 / 128

    property int duration: 300

    ParallelAnimation {
        running: true
        NumberAnimation {
            target: main
            property: "opacity"
            to: 0
            duration: main.duration
        }
        NumberAnimation {
            target: main
            property: "scale"
            to: 1
            duration: main.duration
            easing.type: Easing.OutQuad
        }

        onRunningChanged: {
            if (!running) {
                main.destroy();
            }
        }
    }
}