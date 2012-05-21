/******************************************************************************
 *  Copyright (C) 2012 by Shaun Reich <shaun.reich@blue-systems.com>          *
 *                                                                            *
 *  This library is free software; you can redistribute it and/or modify      *
 *  it under the terms of the GNU Lesser General Public License as published  *
 *  by the Free Software Foundation; either version 2 of the License or (at   *
 *  your option) any later version.                                           *
 *                                                                            *
 *  This library is distributed in the hope that it will be useful,           *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 *  Library General Public License for more details.                          *
 *                                                                            *
 *  You should have received a copy of the GNU Lesser General Public License  *
 *  along with this library; see the file COPYING.LIB.                        *
 *  If not, see <http://www.gnu.org/licenses/>.                               *
 *****************************************************************************/

import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1 as QtExtra

Item {
    id: main

    property int iconWidth: 64
    property QtObject favoriteModel
    property alias currentText: resultLabel.text
    property alias currentIcon: resultIcon.icon;
    property string currentUrl;
    property string currentId;
    //to allow public access to these members..
    property alias favoriteIcon: favoriteIcon
    property alias resultLabel: resultLabel

    property bool isFavorite: favoriteModel.isFavorite(currentUrl)

    signal clicked
    property bool containsMouse: itemMouseArea.containsMouse || favoriteMouseArea.containsMouse
    signal entered
    signal exited

    width: iconWidth * 2
    //FIXME also hardcoded. probably use a text metric
    height: iconWidth + resultLabel.paintedHeight * 2

    opacity: 0

    Component.onCompleted: {
        opacity = 1
        itemMouseArea.clicked.connect(clicked)
    }

    Behavior on opacity {
        NumberAnimation {
            duration: 300
            easing.type: Easing.OutQuad
        }
    }

    QtExtra.QIconItem {
        id: resultIcon

        anchors {
            topMargin: 5
            top: parent.top
            left: parent.left
            right: parent.right
        }

        width: iconWidth
        height: iconWidth
    }

    PlasmaComponents.Label {
        id: resultLabel

        anchors {
            bottomMargin: 6
            top: resultIcon.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            rightMargin: 5
            leftMargin: 5
        }

        clip: true
        smooth: true
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignHCenter
//            wrapMode: Text.WordWrap
    }

    QtExtra.QIconItem {
        id: favoriteIcon

        anchors {
            right: parent.right
            top: parent.top
            rightMargin: 4
            topMargin: anchors.rightMargin
        }

        Behavior on opacity {
            NumberAnimation {
                duration: 250
                easing.type: Easing.OutQuad
            }
        }
        icon: "bookmarks"

        opacity: {
            if (isFavorite) {
                return favoriteMouseArea.containsMouse ? 0.5 : 1;
            } else {
                return favoriteMouseArea.containsMouse ? 1 : (containsMouse ? 0.5 : 0);
            }
        }
        width: 22
        height: width
    }

    QtExtra.QIconItem {
        anchors {
            right: favoriteIcon.right
            bottom: favoriteIcon.bottom
        }

        icon: isFavorite ? "list-remove" : "list-add"
        opacity: favoriteMouseArea.containsMouse ? 1 : 0
        Behavior on opacity {
            NumberAnimation {
                duration: 250
                easing.type: Easing.OutQuad
            }
        }
        width: 16
        height: width
    }

    MouseArea {
        id: itemMouseArea
        anchors.fill: parent
        hoverEnabled: true
    }

    MouseArea {
        // If MouseArea were a child of favoriteIcon it would not work
        // when favoriteIcon.opacity is 0. That's why it is a sibling.
        id: favoriteMouseArea
        anchors.fill: favoriteIcon
        hoverEnabled: true

        onClicked: {
            var url = currentUrl;
            if (isFavorite) {
                favoriteModel.remove(url);
            } else {
                favoriteModel.add(url);
            }
            // Overwrite "isFavorite" property to reflect the change.
            // It is a bit of a hack, but "isFavorite" does not get
            // updated after the change, so we have to update it ourself.
            //
            // We read the result from favoriteModel instead of using
            // !favorite so that if favoriting this item failed for some
            // reason, we do not show wrong favorite status.
            isFavorite = favoriteModel.isFavorite(url);
        }
    }
}
