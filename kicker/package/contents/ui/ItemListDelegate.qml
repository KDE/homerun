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
import org.kde.plasma.components 0.1 as PlasmaComponents

import org.kde.homerun.components 0.1 as HomerunComponents

PlasmaComponents.ListItem {
    height: itemHeight

    signal actionTriggered(string actionId, variant actionArgument)
    signal aboutToShowActionMenu(variant actionMenu)

    property bool hasChildren: (listView.model != undefined && "modelForRow" in listView.model && listView.model.modelForRow(index).count)
    property bool hasActionList: model.favoriteId || (("hasActionList" in model) && model.hasActionList)

    Keys.onPressed: {
        if (!hasActionList) {
            return;
        }

        if (event.key == Qt.Key_M || event.key == Qt.Key_Menu) {
            event.accepted = true;
            openActionMenu(mouseArea);
        } else if ((event.key == Qt.Key_Enter || event.key == Qt.Key_Return) && !hasChildren) {
            if (!hasChildren) {
                listView.model.trigger(index, "");
                plasmoid.hidePopup();
            }
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        property int mouseCol

        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: {
            if (!hasChildren) {
                listView.model.trigger(index, "");
                plasmoid.hidePopup();
            }
        }

        onPressed: {
            if (mouse.buttons & Qt.RightButton && hasActionList) {
                openActionMenu(mouseArea, mouse.x, mouse.y);
            }
        }

        onPositionChanged: {
            //FIXME: correct escape angle calc for right screen edge
            if (justOpenedTimer.running || hasChildren) {
                listView.currentIndex = index;
            } else {
                mouseCol = mouse.x;
                updateCurrentItemTimer.interval = (mouse.x < listView.eligibleWidth) ? 0 : 50;
                updateCurrentItemTimer.start();
            }
        }

        onContainsMouseChanged: {
            if (!containsMouse) {
                updateCurrentItemTimer.stop();
            }
        }

        function updateCurrentItem() {
            listView.currentIndex = index;
            listView.eligibleWidth = Math.min(width, mouseCol + 5);
        }

        Timer {
            id: updateCurrentItemTimer

            repeat: false

            onTriggered: parent.updateCurrentItem()
        }
    }

    HomerunComponents.Image {
        id: icon

        visible: false

        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
        }

        onSourceChanged: {
            if (source) {
                visible = true;
                width = theme.smallIconSize;
                height = theme.smallIconSize;
            } else {
                visible = false;
                width = 0;
                height = 0;
            }
        }

        source: model.decoration
    }

    Text {
        anchors {
            top: parent.top
            left: icon.right
            leftMargin: 6
            bottom: parent.bottom
            right: arrow.left
        }

        height: Math.max(paintedHeight, theme.defaultFont.mSize.height*1.6)

        opacity: enabled? 1 : 0.6

        text: model.display

        color: theme.textColor
        elide: Text.ElideRight
        verticalAlignment: lineCount > 1 ? Text.AlignTop : Text.AlignVCenter
        font.capitalization: theme.defaultFont.capitalization
        font.family: theme.defaultFont.family
        font.italic: theme.defaultFont.italic
        font.letterSpacing: theme.defaultFont.letterSpacing
        font.pointSize: theme.defaultFont.pointSize
        font.strikeout: theme.defaultFont.strikeout
        font.underline: theme.defaultFont.underline
        font.weight: theme.defaultFont.weight
        font.wordSpacing: theme.defaultFont.wordSpacing
    }

    PlasmaCore.SvgItem {
        id: arrow

        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter

        width: theme.smallIconSize
        height: theme.smallIconSize

        visible: (listView.currentIndex == index) && hasChildren

        svg: arrows
        elementId: "right-arrow"
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

        if ("setDesktopContainmentMutable" in listView.model) {
            listView.model.setDesktopContainmentMutable(appletProxy.desktopContainmentMutable());
        }

        if ("setAppletContainmentMutable" in listView.model) {
            listView.model.setAppletContainmentMutable(appletProxy.appletContainmentMutable());
        }

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

        var closeRequested = listView.model.trigger(index, actionId, actionArgument);

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