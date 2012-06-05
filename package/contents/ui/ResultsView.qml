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

FocusScope {
    id: main

    height: childrenRect.height

    signal indexClicked(int index)

    //FIXME: figure out sizing properly..
    property int resultItemHeight: 128
    property int resultItemWidth: 128

    property string currentText: "";
    property string currentIcon: "";

    property QtObject favoriteModel

    property alias model: gridView.model

    property string path: model.path ? model.path : "/"
    onPathChanged: pathModel.update(path)

    PathModel {
        id: pathModel
    }

    // Components
    Component {
        id: highlight

        PlasmaComponents.Highlight {
            id: highlighter
            hover: true
        }
    }

    Component {
        id: result
        Result {
            currentText: model.label
            currentIcon: model.icon
            showFavoriteIcon: model.favoriteAction != ""
            favoriteIcon: {
                switch (GridView.view.model.favoriteAction(model)) {
                case "add":
                    return "bookmarks";
                case "remove":
                    return "list-remove";
                default:
                    return "";
                }
            }

            onClicked: indexClicked(gridView.currentIndex)

            onFavoriteClicked: {
                GridView.view.model.triggerFavoriteAction(model);
                showFeedback();
            }

            onContainsMouseChanged: {
                if (containsMouse) {
                    resultEntered();
                } else {
                    resultExited();
                }
            }

            function resultEntered() {
                if (resultLabel.truncated == true) {
                    // there's not enough room for the result's text to be displayed, so it's
                    // being ellided/truncated. only then should the tooltip be shown.
                    tooltipShowTimer.restart()
                }

                gridView.currentIndex = index
            }

            function resultExited() {
                tooltipShowTimer.restart()
                tooltipShowTimer.running = false

                tooltipDialog.visible = false
                gridView.currentIndex = -1
            }

            Timer {
                id: tooltipShowTimer

                interval: 800
                repeat: false

                onTriggered:   {
                    var point = tooltipDialog.popupPosition(parent)
                    tooltipDialog.x = point.x
                    tooltipDialog.y = point.y
                    tooltipDialog.visible = true
                    tooltipText.text = model["label"]
                }
            }
        }
    }

    // UI
    PlasmaCore.Dialog {
        id: tooltipDialog

        Component.onCompleted: {
            tooltipDialog.setAttribute(Qt.WA_X11NetWmWindowTypeDock, true)
            tooltipDialog.windowFlags |= Qt.WindowStaysOnTopHint|Qt.X11BypassWindowManagerHint
        }

        mainItem: Text {
            id: tooltipText
            text: "THIS IS A TEST TEXT ITEM"
        }
    }

    PlasmaComponents.Label {
        id: headerLabel
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        text: model.name
        width: parent.width
    }

    Row {
        id: breadCrumbRow
        anchors {
            top: headerLabel.bottom
            left: parent.left
            right: parent.right
        }
        visible: main.path != "/"
        spacing: 6

        Repeater {
            model: pathModel
            delegate: PlasmaComponents.ToolButton {
                text: model.text
                onClicked: main.model.path = model.path
            }
        }
    }

    GridView {
        id: gridView
        anchors {
            top: breadCrumbRow.bottom
            left: parent.left
            right: parent.right
        }

        focus: true

        /*
        // Debug help
        LogText {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            text:
                    "activeFocus=" + gridView.activeFocus
                + "\ncurrentIndex=" + gridView.currentIndex
                + "\ncurrentItem.activeFocus=" + (gridView.currentItem ? gridView.currentItem.activeFocus : "-")
                + "\ncurrentItem.label=" + (gridView.currentItem ? gridView.currentItem.currentText : "-")
        }
        */

        // Defining "height" as "contentHeight" would be simpler, but it causes "Binding loop detected" error messages
        height: Math.ceil(count * cellWidth / width) * cellHeight

        // Disable the GridView flickable so that it does not interfer with the global flickable
        interactive: false

        currentIndex: -1

        cellWidth: resultItemWidth
        cellHeight: resultItemHeight
        //TODO: something sane?
        cacheBuffer: 128 * 10 //10 above, 10 below caching

        highlight: highlight
        highlightFollowsCurrentItem: true

        delegate: result

        // Code
        onActiveFocusChanged: {
            if (focus && currentIndex == -1 && count > 0) {
                currentIndex = 0;
            }
        }

        Keys.onPressed: {
            if (event.key == Qt.Key_Left) {
                moveCurrentIndexLeft();
                event.accepted = true;
            } else if (event.key == Qt.Key_Right) {
                moveCurrentIndexRight();
                event.accepted = true;
            } else if (event.key == Qt.Key_Up) {
                moveCurrentIndexUp();
                event.accepted = true;
            } else if (event.key == Qt.Key_Down) {
                moveCurrentIndexDown();
                event.accepted = true;
            }
        }

        Keys.onReturnPressed: indexClicked(currentIndex)
    }
}
