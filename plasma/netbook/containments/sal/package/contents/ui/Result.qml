/******************************************************************************
 *  Copyright (C) 2012 by Shaun Reich <shaun.reich@blue-systems.com>          *
 *                                                                            *
 *  This library is free software; you can redistribute it and/or modify      *
 *  it under the terms of the GNU Lesser General Public License as published  *
 *  by the Free Software Foundation; either version 2 of the License or (at   *
 *  your option) any later version.                                           *
 *                                                                            *
 *  This library is distributed in the hope that it will be useful,           *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 *  Library General Public License for more details.                          *
 *                                                                            *
 *  You should have received a copy of the GNU Lesser General Public License  *
 *  along with this library; see the file COPYING.LIB.                        *
 *  If not, see <http://www.gnu.org/licenses/>.                               *
 *****************************************************************************/

import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1 as QtExtra

Item {
    id: main

    property int iconWidth: 64
    property alias currentText: resultLabel.text
    property alias currentIcon: resultIcon.icon;
    property string currentId;

    property bool wasClicked: false

    width: iconWidth * 2
    //FIXME also hardcoded. probably use a text metric
    height: iconWidth + resultLabel.paintedHeight * 2

    Column {
        anchors.centerIn: parent

        QtExtra.QIconItem {
            id: resultIcon

            anchors {
                horizontalCenter: parent.horizontalCenter
            }

            width: iconWidth
            height: iconWidth
        }

        PlasmaComponents.Label {
            id: resultLabel

            anchors {
                topMargin: 10
                horizontalCenter: parent.horizontalCenter
            }

            width: iconWidth //+ (iconWidth / 2)

            clip: true
            smooth: true
            elide: Text.ElideRight
            wrapMode: Text.WordWrap
        }
    }



    MouseArea {
        anchors.fill: parent

        hoverEnabled: true

        onPressed: {
            highlighter.opacity = 0
            // because if we just change it to true, we won't
            // do anything on a second click
            wasClicked = false
            wasClicked = true
        }
    }
}