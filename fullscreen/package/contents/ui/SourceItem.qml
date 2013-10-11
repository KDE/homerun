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

SlidingContainer {
    id: main
    property QtObject sourceRegistry
    property string sourceId

    property bool configureMode: false

    property bool isFirst: false
    property bool isLast: false

    show: configureMode

    signal removeRequested
    signal moveRequested(int delta)
    signal configureRequested

    PlasmaCore.FrameSvgItem {
        id: frame
        imagePath: "widgets/tooltip"
        width: parent.width
        height: label.height + margins.top + margins.bottom

        PlasmaComponents.Label {
            id: label
            x: parent.margins.left
            y: parent.margins.top
            text: {
                var name = "";
                if (sourceId) {
                    name = sourceRegistry.visibleNameForSource(sourceId);
                    if (!name) {
                        name = i18n("Broken source '%1'", sourceId);
                    }
                }
                return name;
            }
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

            PlasmaComponents.ToolButton {
                id: configureButton
                iconSource: "configure"
                visible: sourceId ? sourceRegistry.isSourceConfigurable(sourceId) : false
                onClicked: main.configureRequested()
            }

            PlasmaComponents.ToolButton {
                id: upButton
                iconSource: "go-up"
                enabled: !main.isFirst
                onClicked: main.moveRequested(-1)
            }

            PlasmaComponents.ToolButton {
                id: downButton
                iconSource: "go-down"
                enabled: !main.isLast
                onClicked: main.moveRequested(1)
            }

            PlasmaComponents.ToolButton {
                id: removeButton
                iconSource: "list-remove"
                onClicked: main.removeRequested()
            }
        }
    }
}
