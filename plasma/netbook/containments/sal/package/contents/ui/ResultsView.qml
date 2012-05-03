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

    //FIXME: figure out sizing properly..
    property int resultItemHeight: 128
    property int resultItemWidth: 128

    property int appIndexToRun: 0
    property string urlToRun: ""
    property string currentText: "";
    property string currentIcon: "";

    property string currentFavoriteUrl: ""
    property variant currentFavoriteIcon
    property string currentFavoriteText: ""

    property alias model: gridView.model

    GridView {
        id: gridView
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        focus: true

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

        Keys.onReturnPressed: {
            print("PRESSED")
        }

        cellWidth: resultItemWidth
        cellHeight: resultItemHeight
        //TODO: something sane?
        cacheBuffer: 128 * 10 //10 above, 10 below caching

        clip: true

        highlight: highlight
        highlightFollowsCurrentItem: true

        PlasmaCore.Dialog {
            id: tooltipDialog

            Component.onCompleted: {
                tooltipDialog.setAttribute(Qt.WA_X11NetWmWindowTypeDock, true)
                tooltipDialog.windowFlags |= Qt.WindowStaysOnTopHint|Qt.X11BypassWindowManagerHint
            }

            mainItem: tooltipText
        }

        //FIXME: it won't respond by resizing when i change the text through the onEntered event.
        //it only abides by what it was given at ctor time. need a way to change that...
        Text {
            id: tooltipText
            text: "THIS IS A TEST TEXT ITEM"
        }

        delegate: Result {
            id: result
            currentText: model["label"]
            currentIcon: model["icon"]
            currentId: model["id"]
            currentUrl: model["url"]

            MouseArea {
                anchors {
                    left: parent.left
                    right: result.favoriteIcon.left
                    top: parent.top
                    bottom: parent.bottom
                }

                hoverEnabled: true

                onEntered: {
                    resultEntered();
                }

                onExited: {
                    tooltipShowTimer.restart()
                    tooltipShowTimer.running = false

                    tooltipDialog.visible = false
                }

                onClicked: {
                    appIndexToRun = gridView.currentIndex;
                    //to emit the changed signal anyways, even though it (only seems to be) the same thing
                    urlToRun = "";
                    urlToRun = model["url"]
                    print("URL CLICKED:" + model["url"]);
                }
            }

            function resultEntered() {
                tooltipShowTimer.restart()

                gridView.currentIndex = index
            }

            Timer {
                id: tooltipShowTimer

                interval: 1500
                repeat: false

                onTriggered:   {
                    var point = tooltipDialog.popupPosition(result)
                    tooltipDialog.x = point.x
                    tooltipDialog.y = point.y
                    tooltipDialog.visible = true
                    tooltipText.text = model["label"]
                }
            }

            MouseArea {
                anchors {
                    left: result.favoriteIcon.left
                    right: parent.right
                    top: parent.top
                    bottom: parent.bottom
                }

                hoverEnabled: true

                onClicked: {
                    currentFavoriteText = result.currentText
                    currentFavoriteIcon = result.currentIcon
                    currentFavoriteUrl = result.currentUrl
                }

                onEntered: {
                    result.favoriteIcon.opacity = 1
                    resultEntered();
                }

                onExited: {
                    result.favoriteIcon.opacity = 0
                }
            }
        }
    }

    PlasmaComponents.ScrollBar {
        id: scrollBar
        flickableItem: gridView
        anchors {
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
    }

    Component {
        id: highlight

        PlasmaComponents.Highlight {
            id: highlighter
            hover: true
        }
    }
}
