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
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: main
    property alias text: textLabel.text
    property alias comment: commentLabel.text

    signal clicked

    height: childrenRect.height

    property int padding: 6

    PlasmaComponents.Label {
        id: textLabel
        anchors {
            verticalCenter: addButton.verticalCenter
            left: parent.left
            right: addButton.left
        }
        font.weight: Font.Bold
    }

    PlasmaComponents.ToolButton {
        id: addButton
        anchors {
            top: parent.top
            topMargin: padding
            right: parent.right
        }
        flat: false
        iconSource: "list-add"
        onClicked: main.clicked()
    }

    PlasmaComponents.Label {
        id: commentLabel

        anchors {
            left: parent.left
            right: parent.right
            top: addButton.bottom
        }

        wrapMode: Text.Wrap
    }

    PlasmaCore.SvgItem {
        anchors {
            left: parent.left
            right: parent.right
            top: commentLabel.bottom
            topMargin: padding
        }
        height: naturalSize.height
        svg: PlasmaCore.Svg {
            imagePath: "widgets/line"
        }
        elementId: "horizontal-line"
    }
}
