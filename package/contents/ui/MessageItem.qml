/*
Copyright 2013 Aurélien Gâteau <agateau@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
import QtQuick 1.1

import org.kde.plasma.core 0.1 as PlasmaCore

import org.kde.homerun.components 0.1 as HomerunComponents

/**
 * An item which is invisible by default but can show a message
 * composed of a text and an icon for a few seconds.
 */
Item {
    id: main
    width: frame.width
    height: frame.height

    opacity: 0
    Behavior on opacity { NumberAnimation {}}

    Timer {
        id: timer
        interval: 5000
        onTriggered: main.state = "";
    }

    PlasmaCore.FrameSvgItem {
        id: frame
        y: -10
        Behavior on y { NumberAnimation {}}
        imagePath: "widgets/tooltip"
        width: row.width + margins.left + margins.right
        height: row.height + margins.top + margins.bottom

        Row {
            id: row
            spacing: 6
            x: frame.margins.left
            y: frame.margins.top
            HomerunComponents.Image {
                id: icon
                width: 22
                height: width
                anchors.verticalCenter: parent.verticalCenter
            }
            HomerunComponents.Label {
                id: label
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    states: State {
        name: "visible"
        PropertyChanges {
            target: main
            opacity: 1
        }
        PropertyChanges {
            target: frame
            y: 0
        }
        PropertyChanges {
            target: timer
            running: true
        }
    }

    function show(iconName, text) {
        icon.source = iconName;
        label.text = text;
        state = "visible";
    }
}
