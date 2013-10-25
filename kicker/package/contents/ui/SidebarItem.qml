/*
 *   Copyright (C) 2013 by Eike Hein <hein@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 1.1

import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.draganddrop 1.0 as DragAndDrop
import org.kde.qtextracomponents 0.1 as QtExtra

import org.kde.homerun.components 0.1 as HomerunComponents
import org.kde.homerun.kicker 0.1 as HomerunKicker

DragAndDrop.DragArea {
    id: sidebarItem

    width: parent.width
    height: width

    signal actionTriggered(string actionId, variant actionArgument)
    signal aboutToShowActionMenu(variant actionMenu)

    property bool edgeItem: (y < parent.height && y + height > parent.height)

    property int itemIndex: model.index
    property bool hasActionList: model.favoriteId || (("hasActionList" in model) && model.hasActionList)

    delegateImage: model.decoration
    supportedActions: Qt.MoveAction

    mimeData {
        source: sidebarItem
    }

    QtExtra.MouseEventListener {
        id: listener

        anchors.fill: parent

        hoverEnabled: true

        onClicked: {
            repeater.model.trigger(index, "", null);
            plasmoid.hidePopup();
        }

        onPressed: {
            if (mouse.buttons & Qt.RightButton && hasActionList) {
                openActionMenu(sidebarItem, mouse.x, mouse.y);
            }
        }
    }

    HomerunKicker.FadeOutItem {
        anchors.fill: parent

        covered: edgeItem ? (sidebarItem.y + height - sidebarItem.parent.height) : 0

        PlasmaCore.IconItem {
            anchors.fill: parent

            active: listener.containsMouse

            source: model.decoration
        }
    }

    HomerunComponents.ActionMenu {
        id: actionMenu

        onActionClicked: {
            actionTriggered(actionId, actionArgument);
        }
    }

    onAboutToShowActionMenu: {
        fillActionMenu(actionMenu);
    }

    onActionTriggered: {
        triggerAction(model.index, actionId, actionArgument);
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

    function triggerAction(index, actionId, actionArgument) {
        // Looks like String.startsWith does not exist in the JS interpreter we use :/
        function startsWith(txt, needle) {
            return txt.substr(0, needle.length) === needle;
        }
        if (startsWith(actionId, "_homerun_favorite_")) {
            handleFavoriteAction(actionId, actionArgument);
            return;
        }

        var closeRequested = repeater.model.trigger(index, actionId, actionArgument);

        if (closeRequested) {
            plasmoid.hidePopup();
        }
    }

    function openActionMenu(visualParent, x, y) {
        aboutToShowActionMenu(actionMenu);
        actionMenu.visualParent = visualParent;
        actionMenu.open(x, y);
    }
}
