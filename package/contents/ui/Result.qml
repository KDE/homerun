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

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1 as QtExtra

import org.kde.homerun.components 0.1 as HomerunComponents

Item {
    id: main

    //- Public ----------------------------------------------------------------
    // Set by others
    property int iconWidth: 64
    property string favoriteIcon
    property alias text: resultLabel.text
    property alias icon: resultIcon.icon
    property bool configureMode: false

    // Exposed by us
    property alias truncated: resultLabel.truncated

    // "highlighted" property
    // This property is controlled by both the mouse and the keyboard. It cannot
    // be represented as containsMouse || activeFocus because it must track the
    // latest event.
    // For example if mouse is over item A and user presses the right arrow
    // button to reach item B, then B.highlighted should become true and
    // A.highlighted should become false, even if the mouse is still over A.
    property bool highlighted: false

    signal clicked
    signal favoriteClicked

    //- Private ---------------------------------------------------------------
    property bool containsMouse: itemMouseArea.containsMouse || favoriteMouseArea.containsMouse
    onContainsMouseChanged: {
        highlighted = containsMouse;
    }
    onActiveFocusChanged: {
        highlighted = activeFocus;
    }

    property int padding: 5
    height: resultIcon.height + resultLabel.paintedHeight + 2 * padding

    Component.onCompleted: {
        itemMouseArea.clicked.connect(clicked)
        favoriteMouseArea.clicked.connect(favoriteClicked)
    }

    Component {
        id: favoriteFeedbackComponent
        FavoriteFeedback {
        }
    }

    function showFeedback() {
        favoriteFeedbackComponent.createObject(favoriteIconItem);
    }

    QtExtra.QIconItem {
        id: resultIcon

        anchors {
            topMargin: main.padding
            top: parent.top
            left: parent.left
            right: parent.right
        }

        width: iconWidth
        height: iconWidth
    }

    HomerunComponents.Label {
        id: resultLabel

        anchors {
            bottomMargin: main.padding
            top: resultIcon.bottom
            left: parent.left
            right: parent.right
            rightMargin: main.padding
            leftMargin: main.padding
        }

        elide: Text.ElideRight
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignTop
        wrapMode: Text.Wrap
        maximumLineCount: 2
    }

    QtExtra.QIconItem {
        id: favoriteIconItem

        anchors {
            right: parent.right
            top: parent.top
            rightMargin: padding
            topMargin: padding
        }

        Behavior on opacity {
            NumberAnimation {
                duration: 250
                easing.type: Easing.OutQuad
            }
        }
        icon: main.favoriteIcon

        // Use "main.favoriteIcon" and not "icon" because right now "icon"
        // is a non-notifiable property
        visible: main.favoriteIcon != ""

        opacity: favoriteMouseArea.containsMouse ? 1 : (main.highlighted ? 0.5 : 0)
        width: 22
        height: width
    }

    MouseArea {
        id: itemMouseArea
        anchors.fill: parent
        hoverEnabled: true
        enabled: !configureMode
    }

    MouseArea {
        // If MouseArea were a child of favoriteIconItem it would not work
        // when favoriteIconItem.opacity is 0. That's why it is a sibling.
        id: favoriteMouseArea
        anchors.fill: favoriteIconItem
        hoverEnabled: true
        enabled: !configureMode
    }
}
