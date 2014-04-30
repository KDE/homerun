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

import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.extras 0.1 as PlasmaExtras
import org.kde.qtextracomponents 0.1 as QtExtra

FocusScope {
    id: itemList

    width: theme.defaultFont.mSize.width * ((nameFormat > 1) ? 20 : 18)
    height: itemDialog.parent.childItemListHeight

    property Item dialog: null
    property QtObject childDialog: null
    property int childItemListHeight

    property bool containsMouse: mouseEventListener.containsMouse || (childDialog != null && childDialog.mainItem.containsMouse)

    property alias model: listView.model
    property alias currentIndex: listView.currentIndex
    property alias currentItem: listView.currentItem

    property int itemHeight: Math.max(theme.defaultFont.mSize.height, theme.smallIconSize) + listItemSvg.margins.top + listItemSvg.margins.bottom

    onFocusChanged: {
        if (focus) {
            main.focusChanged();
        }
    }

    onContainsMouseChanged: {
        if (!containsMouse) {
            resetIndexTimer.start();
        } else {
            resetIndexTimer.stop();
        }
    }

    Timer {
        id: dialogSpawnTimer

        interval: 70
        repeat: false

        onTriggered: {
            if (!plasmoid.popupShowing) {
                return;
            }

            childDialog = itemListDialogComponent.createObject(itemList);
            childDialog.setAttribute(Qt.WA_X11NetWmWindowTypeMenu, true)
            childDialog.visible = true;
        }
    }

    Timer {
        id: resetIndexTimer

        interval: (dialog != null) ? 50 : 150
        repeat: false

        onTriggered: {
            if (focus) {
                currentIndex = -1;
            }
        }
    }

    QtExtra.MouseEventListener {
        id: mouseEventListener

        anchors.fill: parent

        hoverEnabled: true

        onContainsMouseChanged: {
            listView.eligibleWidth = listView.width;
        }

        PlasmaExtras.ScrollArea {
            anchors.fill: parent

            ListView {
                id: listView

                property int eligibleWidth: width

                // HACK: Hack to work around bug in ScrollArea that causes the overflow
                // deco to be visible when it shouldn't be.
                contentHeight: itemList.height - 1

                focus: true

                snapMode: ListView.SnapToItem
                spacing: 0
                keyNavigationWraps: (dialog != null || itemList != sourcesList)

                model: itemList.model
                delegate: ItemListDelegate {}
                highlight: PlasmaComponents.Highlight { anchors.fill: listView.currentItem }

                onCountChanged: {
                    currentIndex = -1;

                    // HACK: https://bugreports.qt-project.org/browse/QTBUG-20927
                    // If you read this: This made me want to cry.
                    for (var i = 0; i < contentItem.children.length; ++i)
                    {
                        contentItem.children[i].y = i * itemHeight;
                    }
                }

                onCurrentIndexChanged: {
                    if (currentIndex != -1) {
                        itemList.focus = true;

                        if (currentItem != null && !currentItem.hasChildren || !plasmoid.popupShowing) {
                            return;
                        }

                        if (childDialog) {
                            childDialog.visible = false;
                            childDialog.delayedDestroy();
                        }

                        if (model != undefined && "modelForRow" in model) {
                            var maxHeight = (Math.floor(windowSystem.workArea().height / itemHeight) - 1) * itemHeight;
                            childItemListHeight = Math.min(model.modelForRow(currentIndex).count * itemHeight, maxHeight);
                        }

                        dialogSpawnTimer.restart();
                    } else if (childDialog != null) {
                        dialogSpawnTimer.stop();
                        childDialog.visible = false;
                        childDialog.delayedDestroy();
                        childDialog = null;
                    }
                }

                Keys.onPressed: {
                    if (event.key == Qt.Key_Right && childDialog != null) {
                        childDialog.mainItem.focus = true;
                        childDialog.mainItem.currentIndex = 0;
                    } else if (event.key == Qt.Key_Left && dialog != null) {
                        dialog.parent.focus = true;
                        dialog.destroy();
                    } else if (event.key != Qt.Key_Escape && event.text != "") {
                        searchField.appendText(event.text);
                    }
                }

                function focusChanged() {
                    if (!itemList.focus && listView.currentIndex != -1 && childDialog == null) {
                        listView.currentIndex = -1;
                    }
                }

                Component.onCompleted: {
                    main.focusChanged.connect(focusChanged);
                }

                Component.onDestruction: {
                    main.focusChanged.disconnect(focusChanged);
                }
            }
        }
    }
}
