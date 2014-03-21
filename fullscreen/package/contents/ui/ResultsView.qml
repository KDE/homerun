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
import org.kde.homerun.components 0.1 as HomerunComponents
import org.kde.homerun.fixes 0.1 as HomerunFixes
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1 as QtExtra

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
    property bool showActionListOverlay: false

    property alias currentItem: gridView.currentItem

    signal triggerActionRequested(int index, string actionId, variant actionArgument)

    signal focusOtherViewRequested(int key, int x)

    signal showMessageRequested(string icon, string text)

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
        triggerAction(0, "", null);
    }

    //- Private -------------------------------------------------
    function focusItemAt(idx) {
        if (idx < 0 || idx >= gridView.count) {
            return;
        }
        gridView.currentIndex = idx;
        gridView.currentItem.forceActiveFocus();
    }
    height: visible ? childrenRect.height : 0
    visible: calculateVisibility()

    function calculateVisibility() {
        if (!model) {
            return false;
        }

        if (!gridView.count) {
            return false;
        }

        if ("hidden" in model && model.hidden) {
            return false;
        }

        return true;
    }

    //FIXME: figure out sizing properly..
    property int iconWidth: 64
    property int resultItemWidth: 128
    property int resultItemHeight: iconWidth + 3 * theme.defaultFont.mSize.height

    // Components
    Component {
        id: result
        FocusScope {
            width: gridView.cellWidth
            height: gridView.cellHeight
            property alias text: resultMain.text
            property alias icon: resultMain.icon

            Result {
                id: resultMain
                focus: true
                iconWidth: main.iconWidth
                width: main.resultItemWidth
                configureMode: main.configureMode
                showActionListOverlay: main.showActionListOverlay
                dragContainer: gridDragContainer
                dragEnabled: ("canMoveRow" in main.model) ? main.model.canMoveRow : false

                text: model.display
                icon: model.decoration
                itemIndex: model.index

                onHighlightedChanged: {
                    if (highlighted) {
                        gridView.currentIndex = model.index;
                    }
                }

                hasActionList: model.favoriteId || (("hasActionList" in model) && model.hasActionList)

                onAboutToShowActionMenu: {
                    fillActionMenu(actionMenu);
                }

                onActionTriggered: {
                    triggerAction(model.index, actionId, actionArgument);
                }

                onShowMessageRequested: {
                    main.showMessageRequested(icon, text);
                }

                function fillActionMenu(actionMenu) {
                    // Accessing actionList can be a costly operation, so we don't
                    // access it until we need the menu
                    var lst = model.hasActionList ? model.actionList : [];
                    var action = createFavoriteAction();
                    if (action) {
                        if (lst.length > 0) {
                            var separator = { "type": "separator" };
                            lst.unshift(action, separator);
                        } else {
                            lst = [action];
                        }
                    }
                    actionMenu.actionList = lst;
                }

                function createFavoriteAction() {
                    var favoriteModel = favoriteModelForFavoriteId(model.favoriteId);
                    if (favoriteModel === null) {
                        return null;
                    }
                    var action = {};
                    if (favoriteModel.isFavorite(model.favoriteId)) {
                        action.text = i18n("Remove from Favorites");
                        action.icon = QIcon("list-remove");
                        action.actionId = "_homerun_favorite_remove";
                    } else {
                        action.text = i18n("Add to Favorites");
                        action.icon = QIcon("bookmark-new");
                        action.actionId = "_homerun_favorite_add";
                    }
                    action.actionArgument = {favoriteId: model.favoriteId, text: model.display};
                    return action;
                }

            }
        }
    }

    // UI
    HomerunComponents.Label {
        id: headerLabel

        width: parent.width
        height: theme.defaultFont.mSize.height * 2.8

        visible: showHeader
        opacity: ("plasmoid" in this) ? 1 : 0.4

        text: main.model ? main.model.name : "-"
        font.pointSize: theme.defaultFont.pointSize * 1.4
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
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

        objectName: "GridView:" + (main.model ? main.model.objectName : "-")

        HomerunComponents.DragContainer {
            id: gridDragContainer
            anchors.fill: parent
            model: main.model
        }

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
        height: Math.ceil(count / Math.floor(width / cellWidth)) * cellHeight

        // Disable the GridView flickable so that it does not interfer with the global flickable
        interactive: false

        cellWidth: resultItemWidth
        cellHeight: resultItemHeight
        //TODO: something sane?
        cacheBuffer: 128 * 10 //10 above, 10 below caching

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

        Keys.onReturnPressed: triggerAction(currentIndex, "", null)
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

    function handleFavoriteAction(actionId, actionArgument) {
        var favoriteId = actionArgument.favoriteId;
        var favoriteModel = favoriteModelForFavoriteId(favoriteId);
        if (actionId == "_homerun_favorite_remove") {
            favoriteModel.removeFavorite(favoriteId);
        } else if (actionId == "_homerun_favorite_add") {
            favoriteModel.addFavorite(favoriteId);
            showMessageRequested("bookmarks", i18n("%1 has been added to your favorites", actionArgument.text));
        } else {
            console.log("Unknown homerun favorite actionId: " + actionId);
        }
    }

    function triggerAction(index, actionId, actionArgument) {
        // Looks like String.startsWith does not exist in the JS interpreter we use :/
        function startsWith(txt, needle) {
            return txt.substr(0, needle.length) === needle;
        }
        if (configureMode) {
            return;
        }
        if (startsWith(actionId, "_homerun_favorite_")) {
            handleFavoriteAction(actionId, actionArgument);
            return;
        }
        triggerActionRequested(index, actionId, actionArgument);
    }
}
