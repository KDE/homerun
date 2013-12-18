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

import org.kde.homerun.fixes 0.1 as HomerunFixes

HomerunFixes.Dialog {
    id: itemDialog

    property Item listView

    property bool leftOfParent: false

    visible: false

    location: Floating
    windowFlags: Qt.WindowStaysOnTopHint

    mainItem: ItemListView {
        id: itemListView

        dialog: itemDialog

        model: (itemDialog.parent.currentIndex != -1 && itemDialog.parent.model != undefined
            && "modelForRow" in itemDialog.parent.model) ?
            itemDialog.parent.model.modelForRow(itemDialog.parent.currentIndex) : undefined

        onModelChanged: {
            if (model != undefined) {
                updatePosition();
            }
        }
    }

    function delayedDestroy() {
        var timer = Qt.createQmlObject('import QtQuick 1.1; Timer { onTriggered: itemDialog.destroy() }', itemDialog);
        timer.interval = 0;
        timer.start();
    }

    function updatePosition() {
        if (parent.currentItem) {
            syncMainItem();

            var workArea = windowSystem.workArea();
            var pos = windowSystem.mapToScreen(parent, parent.currentItem.x, parent.currentItem.y);

            var xPos = pos.x + parent.width + margins.left;

            var workAreaWidth = workArea.x + workArea.width;
            var availableWidth = workAreaWidth - xPos - margins.right;

            if (width > availableWidth) {
                xPos = pos.x - width - margins.right;
                leftOfParent = true;
            } else {
                leftOfParent = false;
            }

            var yPos = pos.y - margins.top;
            var menuHeight = itemListView.height;

            var workAreaHeight = workArea.y + workArea.height;
            var availableHeight = workAreaHeight - yPos;

            if ((menuHeight + margins.bottom) > availableHeight) {
                var adjust = menuHeight - Math.floor((availableHeight - margins.bottom) / itemListView.itemHeight) * itemListView.itemHeight;
                yPos = yPos - adjust;
            }

            x = xPos;
            y = yPos;
        }
    }
}
