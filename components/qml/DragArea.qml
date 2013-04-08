/*
Copyright 2013 Aurélien Gâteau <agateau@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
import QtQuick 1.1

Item {
    id: main
    property Item dragContainer
    property Item draggedItem: parent
    property int itemIndex: -1

    property alias hoverEnabled: mouseArea.hoverEnabled
    property bool dragEnabled: true

    // Read only info
    property bool isHoldDown: mouseArea.holdX > -1
    property bool isDragged: dragContainer.draggedIndex != -1 && itemIndex == dragContainer.draggedIndex

    property alias mouseX: mouseArea.mouseX
    property alias mouseY: mouseArea.mouseY
    property alias containsMouse: mouseArea.containsMouse

    /**
     * type:Qt::MouseButtons accepted buttons
     *
     * MiddleButton is already used for dragging, so you can't react to middle button
     */
    property int acceptedButtons

    /**
     * @param type:MouseEvent mouse Mouse event for the click
     */
    signal clicked(variant mouse)

    signal longPressTimeReached()

    /**
     * Emitted when user pressed the item for a long time and released it.
     * This is a good opportunity to show a context menu.
     */
    signal longPressReleased()

    // Internal
    property Item oldParent

    MouseArea {
        id: mouseArea
        acceptedButtons: main.acceptedButtons | Qt.MiddleButton
        anchors.fill: parent

        property real holdX: -1
        property real holdY: -1

        onPressAndHold: {
            holdX = mouseX;
            holdY = mouseY;
            longPressTimeReached();
        }

        onPressed: {
            if (main.dragEnabled && mouse.button == Qt.MiddleButton) {
                main.startDrag();
                mouse.accepted = true;
            }
        }

        onPositionChanged: {
            function moved(a, b) {
                return Math.abs(a, b) > 16;
            }
            if (!main.dragEnabled) {
                if (isHoldDown) {
                    resetHold();
                }
                return;
            }
            // We only reach this point when drag is enabled
            if (main.isHoldDown && (moved(mouseX, holdX) || moved(mouseY, holdY))) {
                resetHold();
                main.startDrag();
                return;
            }
            if (main.isDragged) {
                main.updateDrag();
            }
        }

        onReleased: {
            if (main.isHoldDown) {
                resetHold();
                longPressReleased();
                return;
            }
            if (isDragged) {
                stopDrag();
            }
        }

        onClicked: {
            main.clicked(mouse);
        }

        function resetHold() {
            holdX = -1;
            holdY = -1;
        }
    }

    function startDrag() {
        if (isDragged) {
            return;
        }
        dragContainer.draggedIndex = itemIndex;
        oldParent = draggedItem.parent;

        // Move draggedItem to dragContainer, keeping its position unchanged
        var coord = dragContainer.mapFromItem(main, 0, 0);
        draggedItem.parent = dragContainer;
        draggedItem.x = coord.x;
        draggedItem.y = coord.y;

        mouseArea.drag.target = draggedItem;
    }

    function updateDrag() {
        var coord = dragContainer.mapFromItem(draggedItem, draggedItem.width / 2, draggedItem.height / 2);
        var newIndex = dragContainer.indexAt(coord.x, coord.y);

        if (dragContainer.draggedIndex != -1 && newIndex != -1 && newIndex != dragContainer.draggedIndex) {
            if ("move" in dragContainer.model) {
                dragContainer.model.move(dragContainer.draggedIndex, newIndex, 1);
            } else if ("moveRow" in dragContainer.model) {
                dragContainer.model.moveRow(dragContainer.draggedIndex, newIndex);
            } else {
                console.log("DragArea.updateDrag(): ERROR: Don't know how to move rows for model " + dragContainer.model);
                stopDrag();
                return;
            }
            dragContainer.draggedIndex = newIndex;
        }
    }

    function stopDrag() {
        dragContainer.draggedIndex = -1;

        // Move draggedItem back to its old parent
        draggedItem.parent = oldParent;
        draggedItem.x = 0;
        draggedItem.y = 0;
        oldParent = null;

        mouseArea.drag.target = null;
    }
}
