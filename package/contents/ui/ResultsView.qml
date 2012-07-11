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

    property variant favoriteModels

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
            hover: true
            opacity: gridView.currentItem.highlighted ? 1 : 0
        }
    }

    Component {
        id: result
        Result {
            id: main
            currentText: model.label
            currentIcon: model.icon
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

            onClicked: indexClicked(model.index)

            onFavoriteClicked: {
                var favoriteModel = favoriteModelForFavoriteId(model.favoriteId);
                if (favoriteModel.isFavorite(model.favoriteId)) {
                    favoriteModel.removeFavorite(model.favoriteId);
                } else {
                    favoriteModel.addFavorite(model.favoriteId);
                }
                showFeedback();
            }
        }
    }

    // UI
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

        cellWidth: resultItemWidth
        cellHeight: resultItemHeight
        //TODO: something sane?
        cacheBuffer: 128 * 10 //10 above, 10 below caching

        highlight: highlight

        delegate: result

        Keys.onPressed: {
            // We must handle key presses ourself because we set interactive to false
            var oldIndex = currentIndex;
            if (event.key == Qt.Key_Left) {
                moveCurrentIndexLeft();
            } else if (event.key == Qt.Key_Right) {
                moveCurrentIndexRight();
            } else if (event.key == Qt.Key_Up) {
                moveCurrentIndexUp();
            } else if (event.key == Qt.Key_Down) {
                moveCurrentIndexDown();
            }
            // Only accept the event if the index actually moved. Not accepting
            // it will cause parent items to move the focus to the next ResultsView,
            // which is what we want
            event.accepted = currentIndex != oldIndex;
        }

        Keys.onReturnPressed: indexClicked(currentIndex)
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
                label.text = target.currentText;
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
}
