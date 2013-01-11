/*
Copyright 2012 Aurélien Gâteau <agateau@kde.org>

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
import org.kde.qtextracomponents 0.1 as QtExtra

QtExtra.QIconItem {
    id: main
    anchors.centerIn: parent
    icon: "bookmarks"
    width: 128
    height: width
    scale: 32 / 128

    property int duration: 300

    ParallelAnimation {
        running: true
        NumberAnimation {
            target: main
            property: "opacity"
            to: 0
            duration: main.duration
        }
        NumberAnimation {
            target: main
            property: "scale"
            to: 1
            duration: main.duration
            easing.type: Easing.OutQuad
        }

        onRunningChanged: {
            if (!running) {
                main.destroy();
            }
        }
    }
}