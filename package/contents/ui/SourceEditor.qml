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
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents

Column {
    id: main
    property QtObject sourceRegistry
    property string sourceId

    property bool configureMode: false

    property bool isFirst: false
    property bool isLast: false

    height: configureMode ? Math.min(200, childrenRect.height) : childrenRect.height

    clip: configureMode

    signal removeRequested
    signal moveRequested(int delta)
    signal sourceIdChanged(string sourceId)

    PlasmaCore.FrameSvgItem {
        id: frame
        imagePath: "widgets/tooltip"
        width: parent.width
        height: configureMode ? (label.height + margins.top + margins.bottom) : 0
        opacity: configureMode ? 1 : 0

        Behavior on height {
            NumberAnimation {}
        }
        Behavior on opacity {
            NumberAnimation {}
        }

        PlasmaComponents.Label {
            id: label
            x: parent.margins.left
            y: parent.margins.top
            text: sourceId ? sourceRegistry.visibleNameForSource(sourceId) : ""
        }

        Row {
            id: buttonRow
            height: label.height
            anchors {
                right: parent.right
                rightMargin: parent.margins.right
                top: parent.top
                topMargin: parent.margins.top
            }

            PlasmaComponents.Button {
                id: configureButton
                width: height
                iconSource: "configure"
                visible: sourceId ? sourceRegistry.isSourceConfigurable(sourceId) : false
                onClicked: {
                    var dlg = sourceRegistry.createConfigurationDialog(sourceId);
                    if (dlg.exec()) {
                        sourceIdChanged(dlg.sourceId());
                    }
                    dlg.destroy();
                }
            }

            PlasmaComponents.Button {
                id: upButton
                width: height
                iconSource: "arrow-up"
                enabled: !main.isFirst
                onClicked: main.moveRequested(-1)
            }

            PlasmaComponents.Button {
                id: downButton
                width: height
                iconSource: "arrow-down"
                enabled: !main.isLast
                onClicked: main.moveRequested(1)
            }

            PlasmaComponents.Button {
                id: removeButton
                width: height
                iconSource: "list-remove"
                onClicked: main.removeRequested()
            }
        }
    }
}
