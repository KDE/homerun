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
    property alias icon: resultIcon.source
    property bool configureMode: false
    property bool showActionListOverlay: false
    property bool hasActionList: false
    property alias dragEnabled: dragArea.dragEnabled

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

    property alias dragContainer: dragArea.dragContainer
    property alias itemIndex: dragArea.itemIndex

    signal actionTriggered(string actionId, variant actionArgument)
    signal aboutToShowActionMenu(variant actionMenu)

    /**
     * Emitted when the item wants to show a global message, for example to
     * indicate possible actions when it is held down.
     */
    signal showMessageRequested(string icon, string text)

    //- Private ---------------------------------------------------------------
    property bool containsMouse: dragArea.containsMouse || (showActionListOverlay && actionListMouseArea.containsMouse)
    onContainsMouseChanged: {
        highlighted = containsMouse;
    }
    onActiveFocusChanged: {
        highlighted = activeFocus;
    }

    property int padding: 5
    height: resultIcon.height + resultLabel.paintedHeight + 3 * padding

    opacity: (dragArea.isDragged || dragArea.isHoldDown) ? 0.6 : 1
    Behavior on opacity { NumberAnimation {} }

    PlasmaCore.FrameSvgItem {
        id: background
        anchors {
            fill: parent
        }

        imagePath: "widgets/viewitem"
        prefix: "hover"

        opacity: !dragArea.isDragged && (main.highlighted || actionMenu.opened) ? 1 : 0

        Behavior on opacity {
            NumberAnimation {
                duration: 250
                easing.type: Easing.OutQuad
            }
        }
    }

    HomerunComponents.Image {
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
            top: resultIcon.bottom
            left: parent.left
            right: parent.right
            bottomMargin: main.padding
            topMargin: main.padding
            rightMargin: main.padding
            leftMargin: main.padding
        }

        elide: Text.ElideRight
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignTop
        wrapMode: Text.Wrap
        maximumLineCount: 2
    }

    HomerunComponents.DragArea {
        id: dragArea
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

        onLongPressTimeReached: {
            var icon = "dialog-information";
            var text;
            if (hasActionList && dragEnabled) {
                text = i18n("Drag to reorder; release to show menu");
            } else if (hasActionList) {
                text = i18n("Release to show menu");
            } else if (dragEnabled) {
                text = i18n("Drag to reorder");
            } else {
                icon = "dialog-error";
                text = i18n("This item has no menu and cannot be dragged");
            }
            showMessageRequested(icon, text);
        }

        onLongPressReleased: {
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
        visible: showActionListOverlay && hasActionList

        PlasmaComponents.ToolButton {
            id: actionListButton
            iconSource: "go-down"
            anchors.fill: parent
            // Hack to get a background on mouse-over only
            flat: !actionListMouseArea.containsMouse
        }

        QtExtra.MouseEventListener {
            id: actionListMouseArea
            anchors.fill: parent
            hoverEnabled: true
            enabled: !configureMode && hasActionList
            onClicked: openActionMenu(actionListButton)
        }
    }


    HomerunComponents.ActionMenu {
        id: actionMenu
        onActionClicked: {
            main.actionTriggered(actionId, actionArgument);
        }
    }

    Keys.onPressed: {
        if (!hasActionList) {
            return;
        }
        if (event.key == Qt.Key_M || event.key == Qt.Key_Menu) {
            event.accepted = true;
            openActionMenu(main);
        }
    }

    // Code
    function openActionMenu(visualParent) {
        aboutToShowActionMenu(actionMenu);
        actionMenu.visualParent = visualParent;
        actionMenu.open();
    }
}
