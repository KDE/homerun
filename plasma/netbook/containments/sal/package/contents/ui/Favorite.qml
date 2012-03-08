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
    property bool wasRemoved: false

    width: iconWidth * 2
    //FIXME also hardcoded. probably use a text metric
    height: iconWidth + resultLabel.paintedHeight * 2

    opacity: 1

    Component.onCompleted: {
        opacity = 1
    }

    Behavior on opacity {
        NumberAnimation { duration: 200 }
    }

    QtExtra.QIconItem {
        id: resultIcon

        anchors {
            topMargin: 5
            rightMargin: 40
            top: parent.top
            left: parent.left
            right: parent.right
        }

        width: iconWidth
        height: iconWidth
    }

    QtExtra.QIconItem {
        id: removeIcon

        anchors {
            topMargin: 10
            top: parent.top
            left: resultIcon.right
            right: parent.right
        }

        width: 32
        height: 32
        icon: "list-remove"

        MouseArea {
            anchors.fill: parent

            onClicked: {
                print("LIST ITEM REMOVED CLICKED")
                wasRemoved = true
            }
        }
    }

    PlasmaComponents.Label {
        id: resultLabel

        anchors {
            bottomMargin: 6
            top: resultIcon.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            rightMargin: 5
            leftMargin: 5
        }

        clip: true
        smooth: true
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignHCenter
//            wrapMode: Text.WordWrap
    }

    MouseArea {
        anchors {
            left: resultIcon.left
            top: resultIcon.top
            bottom: resultIcon.bottom
            right: removeIcon.left
        }

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