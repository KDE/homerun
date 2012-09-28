/*
 *   Copyright 2012 Aurélien Gâteau <agateau@kde.org>
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

Item {
    id: main

    //- Defined by outside world -----------------------------------
    property string searchCriteria

    // Defined for pages with a single view on a browsable model
    property QtObject pathModel

    property list<QtObject> models

    //- Private properties -----------------------------------------
    property alias viewContainer: column

    function getFirstView() {
        var lst = KeyboardUtils.findTabMeChildren(this);
        return lst.length > 0 ? lst[0] : null;
    }

    function updateRunning() {
        for (var idx = 0; idx < models.length; ++idx) {
            if (models[idx].running) {
                busyIndicator.running = true;
                return;
            }
        }
        busyIndicator.running = false;
    }

    PlasmaComponents.BusyIndicator {
        id: busyIndicator
        anchors {
            horizontalCenter: parent.horizontalCenter
        }
        y: 12

        opacity: running ? 0.5 : 0
    }

    Flickable {
        id: flickable
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
            right: scrollBar.left
        }
        contentWidth: width
        contentHeight: column.height
        clip: true
        Column {
            id: column
            width: parent.width
        }
    }

    PlasmaComponents.ScrollBar {
        id: scrollBar
        flickableItem: flickable
        anchors {
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
    }

    Behavior on opacity {
        NumberAnimation {
            duration: 200
        }
    }

    Component {
        id: runningConnectionComponent
        Connections {
            ignoreUnknownSignals: true
            onRunningChanged: main.updateRunning()
        }
    }

    function finishModelConnections() {
        for (var idx = 0; idx < models.length; ++idx) {
            var model = models[idx];
            runningConnectionComponent.createObject(main, {"target": model});
        }
        main.updateRunning();
    }
}
