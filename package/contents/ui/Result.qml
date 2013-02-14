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

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1 as QtExtra

import org.kde.homerun.components 0.1 as HomerunComponents

Item {
    id: main

    //- Public ----------------------------------------------------------------
    // Set by others
    property int iconWidth: 64
    property alias text: resultLabel.text
    property alias icon: resultIcon.icon
    property bool configureMode: false
    property bool hasActionList: false

    // Exposed by us
    property alias truncated: resultLabel.truncated

    // "highlighted" property
    // This property is controlled by both the mouse and the keyboard. It cannot
    // be represented as containsMouse || activeFocus because it must track the
    // latest event.
    // For example if mouse is over item A and user presses the right arrow
    // button to reach item B, then B.highlighted should become true and
    // A.highlighted should become false, even if the mouse is still over A.
    property bool highlighted: false

    signal actionTriggered(string actionId, variant actionArgument)
    signal aboutToShowActionMenu(variant actionMenu)

    //- Private ---------------------------------------------------------------
    property bool containsMouse: itemMouseArea.containsMouse || actionListMouseArea.containsMouse
    onContainsMouseChanged: {
        highlighted = containsMouse;
    }
    onActiveFocusChanged: {
        highlighted = activeFocus;
    }

    property int padding: 5
    height: resultIcon.height + resultLabel.paintedHeight + 2 * padding

    PlasmaCore.FrameSvgItem {
        id: background
        anchors {
            fill: parent
        }

        imagePath: "widgets/viewitem"
        prefix: "hover"

        opacity: (main.highlighted || actionMenu.opened) ? 1 : 0

        Behavior on opacity {
            NumberAnimation {
                duration: 250
                easing.type: Easing.OutQuad
            }
        }
    }

    QtExtra.QIconItem {
        id: resultIcon

        anchors {
            topMargin: main.padding
            top: parent.top
            left: parent.left
            right: parent.right
        }

        width: iconWidth
        height: iconWidth
    }

    HomerunComponents.Label {
        id: resultLabel

        anchors {
            bottomMargin: main.padding
            top: resultIcon.bottom
            left: parent.left
            right: parent.right
            rightMargin: main.padding
            leftMargin: main.padding
        }

        elide: Text.ElideRight
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignTop
        wrapMode: Text.Wrap
        maximumLineCount: 2
    }

    MouseArea {
        id: itemMouseArea
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        anchors.fill: parent
        hoverEnabled: true
        enabled: !configureMode
        onClicked: {
            if (mouse.button == Qt.LeftButton) {
                actionTriggered("", null);
            } else if (mouse.button == Qt.RightButton && hasActionList) {
                openActionMenu(main);
            }
        }
        onPressAndHold: {
            if (hasActionList) {
                openActionMenu(main);
            }
        }
    }

    Item {
        anchors {
            right: parent.right
            rightMargin: main.padding
            top: parent.top
            topMargin: main.padding
        }
        width: 16
        height: width
        opacity: actionListMouseArea.containsMouse ? 1 : ((main.highlighted || actionMenu.opened) ? 0.5 : 0)
        Behavior on opacity { NumberAnimation {} }
        visible: hasActionList

        PlasmaComponents.ToolButton {
            id: actionListButton
            iconSource: "go-down"
            anchors.fill: parent
            // Hack to get a background on mouse-over only
            flat: !actionListMouseArea.containsMouse
        }

        MouseArea {
            id: actionListMouseArea
            anchors.fill: parent
            hoverEnabled: true
            enabled: !configureMode && hasActionList
            onClicked: openActionMenu(actionListButton)
        }
    }

    ActionMenu {
        id: actionMenu
        onActionClicked: {
            main.actionTriggered(actionId, actionArgument);
        }
    }

    // Code
    function openActionMenu(visualParent) {
        aboutToShowActionMenu(actionMenu);
        actionMenu.visualParent = visualParent;
        actionMenu.open();
    }
}
