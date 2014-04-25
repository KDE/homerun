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
                listView.model.trigger(index, "", null);
                plasmoid.hidePopup();
            }
        }
    }

    MouseArea {
        id: mouseArea

        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
        }

        height: itemHeight

        property int mouseCol

        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: {
            if (!hasChildren) {
                listView.model.trigger(index, "", null);
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
            if (justOpenedTimer.running || !hasChildren) {
                listView.currentIndex = index;
            } else {
                mouseCol = mouse.x;

                var switchNow = false;

                if (index == currentIndex) {
                    updateCurrentItem();
                } else if ((index == currentIndex - 1) && mouse.y < (itemHeight - 6)
                    || (index == currentIndex + 1) && mouse.y > 5) {

                    if ((childDialog != null && childDialog.leftOfParent)
                        ? mouse.x > listView.eligibleWidth - 5 : mouse.x < listView.eligibleWidth + 5) {
                        updateCurrentItem();
                    }
                } else if ((childDialog != null && childDialog.leftOfParent)
                    ? mouse.x > listView.eligibleWidth : mouse.x < listView.eligibleWidth) {
                    updateCurrentItem();
                }

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
            listView.eligibleWidth = Math.min(width, mouseCol);
        }

        Timer {
            id: updateCurrentItemTimer

            interval: 50
            repeat: false

            onTriggered: parent.updateCurrentItem()
        }
    }

    Row {
        anchors {
            fill: parent
            leftMargin: icon.visible ? 2 : 5
        }

        LayoutMirroring.enabled: (Qt.application.layoutDirection == Qt.RightToLeft)

        spacing: 5

        PlasmaCore.IconItem {
            id: icon

            anchors {
                verticalCenter: parent.verticalCenter
            }

            width: 0
            height: 0

            visible: false

            active: mouseArea.containsMouse

            onSourceChanged: {
                if (source && valid) {
                    visible = true;
                    width = theme.smallIconSize;
                    height = theme.smallIconSize;
                }
            }

            source: model.decoration
        }

        Text {
            width: parent.width - icon.width - icon.anchors.leftMargin - (arrow.visible ? arrow.width : 0) - (arrow.visible ? parent.spacing : 0) - (icon.visible ? parent.spacing : 0)
            height: parent.height

            text: generateText()

            color: theme.textColor
            elide: Text.ElideRight
            verticalAlignment: lineCount > 1 ? Text.AlignTop : Text.AlignVCenter
            horizontalAlignment: (Qt.application.layoutDirection == Qt.RightToLeft) ? Text.AlignRight : Text.AlignLeft
            font.capitalization: theme.defaultFont.capitalization
            font.family: theme.defaultFont.family
            font.italic: theme.defaultFont.italic
            font.letterSpacing: theme.defaultFont.letterSpacing
            font.pointSize: theme.defaultFont.pointSize
            font.strikeout: theme.defaultFont.strikeout
            font.underline: theme.defaultFont.underline
            font.weight: theme.defaultFont.weight
            font.wordSpacing: theme.defaultFont.wordSpacing

            function generateText() {
                if (nameFormat == 0) {
                    return model.display;
                } else if ("genericName" in model && model.genericName != "" && model.genericName != model.display) {
                    if (nameFormat == 1) {
                        return model.genericName;
                    } else if (nameFormat == 2) {
                        return i18nc("App name (Generic name)", "%1 (%2)", model.display, model.genericName);
                    } else {
                        return i18nc("Generic Name (App name)", "%1 (%2)", model.genericName, model.display);
                    }
                }

                return model.display;
            }
        }

        PlasmaCore.SvgItem {
            id: arrow

            anchors {
                verticalCenter: parent.verticalCenter
            }

            width: theme.smallIconSize
            height: theme.smallIconSize

            visible: (listView.currentIndex == index) && hasChildren

            svg: arrows
            elementId: (Qt.application.layoutDirection == Qt.RightToLeft) ? "left-arrow" : "right-arrow"
        }
    }

    HomerunComponents.ActionMenu {
        id: actionMenu

        onActionClicked: {
            actionTriggered(actionId, actionArgument);
        }

        onOpenedChanged: {
            if (!opened) {
                windowSystem.raiseParentWindow(listView);
            }
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

        if ("containment" in listView.model) {
            listView.model.containment = appletProxy.containment;
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
