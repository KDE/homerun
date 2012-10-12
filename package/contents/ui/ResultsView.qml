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
    property alias count: gridView.count
    property bool tabMe: gridView.count > 0

    property bool configureMode: false

    signal indexClicked(int index)

    //- Private -------------------------------------------------
    height: childrenRect.height

    opacity: configureMode ? 0.6 : 1

    //FIXME: figure out sizing properly..
    property int iconWidth: 64
    property int resultItemWidth: 128
    property int resultItemHeight: iconWidth + 3 * theme.defaultFont.mSize.height

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
            id: resultMain
            iconWidth: main.iconWidth
            width: main.resultItemWidth
            configureMode: main.configureMode

            currentText: model.display
            currentIcon: model.decoration
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

        /*
        // Focus debug help
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
                }
                if (currentIndex != oldIndex) {
                    // Only accept the event if the index actually moved. Not accepting
                    // it will cause parent items to move the focus to the next ResultsView,
                    // which is what we want
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
    onCountChanged: {
        function focusFirstNotEmpty(lst, begin, end) {
            for(var idx = begin; idx < end; ++idx) {
                if (lst[idx].count > 0) {
                    lst[idx].forceActiveFocus();
                    return true;
                }
            }
            return false;
        }

        if (count == 0 && activeFocus) {
            // When we had activeFocus but our count comes to 0 (for example because of filtering)
            // try to put focus on another view. Look first for views after us. Then look at views
            // before us.
            var lst = KeyboardUtils.findTabMeChildren(main.parent);
            var idx = lst.indexOf(main);
            if (idx == -1) {
                console.log("ERROR: Cannot find current view in tabMeChildren");
                return;
            }
            if (focusFirstNotEmpty(lst, idx + 1, lst.length)) {
                return;
            }
            focusFirstNotEmpty(lst, 0, idx);
        }
    }

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
