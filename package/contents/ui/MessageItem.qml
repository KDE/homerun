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
import org.kde.plasma.components 0.1 as PlasmaComponents

/**
 * Invisible by default, when show() is called, show a message composed of
 * a text and an icon for a few seconds and fades out afterwards.
 */
PlasmaComponents.Label {
    id: main
    opacity: 0
    Behavior on opacity { NumberAnimation {}}
    Timer {
        id: timer
        interval: 2000
        onTriggered: main.state = "";
    }
    states: State {
        name: "visible"
        PropertyChanges {
            target: main
            opacity: 1
        }
        PropertyChanges {
            target: timer
            running: true
        }
    }

    function show(icon, text) {
        main.text = text;
        state = "visible";
    }
}
