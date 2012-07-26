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

Item {
    id: main

    // Public
    property alias text: innerText.text
    property bool isFirst: false
    property bool isCurrent: false
    signal clicked

    // Private
    width: row.width

    Row {
        id: row
        height: parent.height

        PlasmaComponents.Label {
            id: arrow
            height: parent.height
            visible: !isFirst

            verticalAlignment: Text.AlignVCenter
            text: "  >  "
            font.pointSize: theme.defaultFont.pointSize * 1.2
        }

        PlasmaComponents.Label { 
            id: innerText
            height: parent.height

            verticalAlignment: Text.AlignVCenter
            font.underline: !main.isCurrent
            font.pointSize: theme.defaultFont.pointSize * 1.2
            color: main.isCurrent ? theme.textColor : theme.linkColor

            MouseArea {
                enabled: !main.isCurrent
                anchors.fill: parent
                onClicked: main.clicked()
            }
        }
    }
}
