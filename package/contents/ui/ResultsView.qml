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
import org.kde.homerun.fixes 0.1 as HomerunFixes
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1 as QtExtra

import "KeyboardUtils.js" as KeyboardUtils

FocusScope {
    id: main

    //- Public --------------------------------------------------
    property variant favoriteModels

    property bool showHeader: true
    /* We intentionally do not use an alias for "model" here. With an alias,
     * headerLabel cannot access model.name of sub models returned by
     * RunnerModel.
     */
    property QtObject model

    property bool configureMode: false

    property alias currentItem: gridView.currentItem

    signal indexClicked(int index)

    signal focusOtherViewRequested(int key, int x)

    function isEmpty() {
        return gridView.count == 0;
    }

    function focusLastItem() {
        focusItemAt(gridView.count - 1);
    }

    function focusFirstItem() {
        focusItemAt(0);
    }

    function focusLastItemAtX(x) {
        for (var y = gridView.height - gridView.cellHeight / 2; y > 0; y -= gridView.cellHeight) {
            var idx = gridView.indexAt(x, y);
            if (idx != -1) {
                focusItemAt(idx);
                return;
            }
        }
        focusLastItem();
    }

    function focusFirstItemAtX(x) {
        var idx = gridView.indexAt(x, gridView.cellHeight / 2);
        if (idx != -1) {
            focusItemAt(idx);
        } else {
            focusLastItem();
        }
    }

    function focusedItem() {
        if (currentItem && currentItem.activeFocus) {
            return currentItem;
        } else {
            return null
        }
    }

    function triggerFirstItem() {
        emitIndexClicked(0);
    }

    //- Private -------------------------------------------------
    function focusItemAt(idx) {
        if (idx < 0 || idx >= gridView.count) {
            return;
        }
        // Reset currentIndex so that the highlight is not animated from the
        // previous position
        gridView.currentIndex = -1;
        gridView.currentIndex = idx;
        gridView.currentItem.forceActiveFocus();
    }
    height: childrenRect.height

    opacity: configureMode ? 0.6 : 1
    visible: gridView.count > 0 || configureMode

    //FIXME: figure out sizing properly..
    property int iconWidth: 64
    property int resultItemWidth: 128
    property int resultItemHeight: iconWidth + 3 * theme.defaultFont.mSize.height

    // Components
    Component {
        id: highlight
        PlasmaComponents.Highlight {
            hover: true
            opacity: (gridView.currentItem && gridView.currentItem.highlighted) ? 1 : 0
        }
    }

    Component {
        id: result
        Result {
            id: resultMain
            iconWidth: main.iconWidth
            width: main.resultItemWidth
            configureMode: main.configureMode

            text: model.display
            icon: model.decoration
            favoriteIcon: {
                var favoriteModel = favoriteModelForFavoriteId(model.favoriteId);
                if (favoriteModel === null) {
                    return "";
                }
                return favoriteModel.isFavorite(model.favoriteId) ? "list-remove" : "bookmarks";
            }

            onHighlightedChanged: {
                if (highlighted) {
                    gridView.currentIndex = model.index;
                }
            }

            onClicked: emitIndexClicked(model.index)

            onFavoriteClicked: {
                var favoriteModel = favoriteModelForFavoriteId(model.favoriteId);
                if (favoriteModel.isFavorite(model.favoriteId)) {
                    favoriteModel.removeFavorite(model.favoriteId);
                } else {
                    favoriteModel.addFavorite(model.favoriteId);
                }
                showFeedback();
            }

            onPressAndHold: {
                dragArea.startDrag();
            }

            HomerunFixes.DragArea {
                id: dragArea
                delegate: Item {
                    // Wrap QIconItem in an item, otherwise
                    // DeclarativeDragArea::startDrag thinks its size is 0x0
                    width: resultMain.iconWidth
                    height: width
                    QtExtra.QIconItem {
                        icon: resultMain.icon
                        anchors.fill: parent
                    }
                }
                supportedActions: Qt.CopyAction
                mimeData {
                    text: model.display
                    source: parent
                }
            }
        }
    }

    // UI
    PlasmaComponents.Label {
        id: headerLabel

        width: parent.width
        height: theme.defaultFont.mSize.height * 2.8

        visible: showHeader
        opacity: 0.6

        text: model.name
        font {
            pointSize: theme.defaultFont.pointSize * 1.4
        }
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        PlasmaCore.SvgItem {
            width: parent.width
            height: naturalSize.height
            svg: PlasmaCore.Svg {
                imagePath: "widgets/line"
            }
            elementId: "horizontal-line"
        }
    }

    GridView {
        id: gridView
        anchors {
            top: showHeader ? headerLabel.bottom : parent.top
            left: parent.left
            right: parent.right
        }

        model: main.model
        focus: true

        objectName: "GridView:" + main.model.objectName

        /*
        // Focus debug help
        LogText {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            text:
                    "activeFocus=" + gridView.activeFocus
                + "\ncurrentIndex=" + gridView.currentIndex
                + "\ncurrentItem.activeFocus=" + (gridView.currentItem ? gridView.currentItem.activeFocus : "-")
                + "\ncurrentItem.label=" + (gridView.currentItem ? gridView.currentItem.text : "-")
        }
        */

        // Defining "height" as "contentHeight" would be simpler, but it causes "Binding loop detected" error messages
        height: Math.ceil(count * cellWidth / width) * cellHeight

        // Disable the GridView flickable so that it does not interfer with the global flickable
        interactive: false

        cellWidth: resultItemWidth
        cellHeight: resultItemHeight
        //TODO: something sane?
        cacheBuffer: 128 * 10 //10 above, 10 below caching

        highlight: highlight

        delegate: result

        Keys.onPressed: {
            // We must handle arrow key navigation ourself because 'interactive' is false
            if (event.modifiers == Qt.NoModifier) {
                var oldIndex = currentIndex;
                if (event.key == Qt.Key_Left) {
                    moveCurrentIndexLeft();
                } else if (event.key == Qt.Key_Right) {
                    moveCurrentIndexRight();
                } else if (event.key == Qt.Key_Up) {
                    moveCurrentIndexUp();
                } else if (event.key == Qt.Key_Down) {
                    moveCurrentIndexDown();
                } else {
                    return;
                }
                if (currentIndex == oldIndex) {
                    // We didn't move, ask to move to another view
                    focusOtherViewRequested(event.key, currentItem ? currentItem.x : 0);
                }
                if (currentIndex != oldIndex || !activeFocus) {
                    // Only accept the event if we moved. Otherwise one can't
                    // press Up from the first row to focus the search field.
                    event.accepted = true;
                }
            }
        }

        Keys.onReturnPressed: emitIndexClicked(currentIndex)
    }

    PlasmaCore.FrameSvgItem {
        id: tooltip
        imagePath: "widgets/tooltip"
        property Item target: (gridView.currentItem && gridView.currentItem.highlighted && gridView.currentItem.truncated) ? gridView.currentItem : null
        width: label.width + margins.left + margins.right
        height: label.height + margins.top + margins.bottom

        opacity: target ? 1 : 0

        onTargetChanged: {
            if (target) {
                // Manually update these properties so that they do not get reset as soon as target becomes null:
                // we don't want the properties to be updated then because we need to keep the old text and coordinates
                // while the tooltip is fading out.
                label.text = target.text;
                x = tooltipX();
                y = target.y;
            }
        }

        PlasmaComponents.Label {
            id: label
            x: parent.margins.left
            y: parent.margins.top
        }

        Behavior on opacity {
            NumberAnimation { duration: 250; }
        }

        function tooltipX() {
            var left = gridView.mapToItem(main, target.x, 0).x;
            var value = left + (target.width - width) / 2;
            if (value < 0) {
                return 0;
            }
            if (value > gridView.width - width) {
                return gridView.width - width;
            }
            return value;
        }
    }

    // Code
    function favoriteModelForFavoriteId(favoriteId) {
        if (favoriteId === undefined || favoriteId === "") {
            return null;
        }
        var lst = favoriteId.split(":");
        if (lst.length === 0) {
            return null;
        }
        var model = favoriteModels[lst[0]];
        if (model === undefined) {
            console.log("favoriteModelForFavoriteId(): No favorite model for favoriteId '" + favoriteId + "'");
            return null;
        } else {
            return model;
        }
    }

    function emitIndexClicked(index) {
        if (configureMode) {
            return;
        }
        indexClicked(index);
    }
}
