/*
 * Copyright 2012 Aurélien Gâteau <agateau@kde.org>
 *
 * Based on TabBar.qml from KDE Plasma Components.
 *
 *- Start of KDE Plasma Components TabBar.qml copyright header ---------
 *
 * Copyright 2011 Marco Martin <mart@kde.org>
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * Contact: Nokia Corporation (qt-info@nokia.com)
 *
 * This file is part of the Qt Components project.
 *
 * $QT_BEGIN_LICENSE:BSD$
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
 *     the names of its contributors may be used to endorse or promote
 *     products derived from this software without specific prior written
 *     permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 * $QT_END_LICENSE$
 *
 *- End of KDE Plasma Components TabBar.qml copyright header ------------
 */

/**Documented API
Inherits:
        Item

Imports:
        QtQuick 1.1
        AppManager.js

Description:
        A simple tab button which is using the  plasma theme.

Properties:
        Item tab:
        The reference to the tab content (one of the children of a TabGroup,
        usually a Page) that is activated when this TabButton is clicked.

        bool checked:
        True if the button is checked,otherwise false.

        bool pressed:
        True if the button is being pressed,otherwise false.

        string text:
        Sets the text for the button.

        string iconSource:
        Icon for the button. It can be a Freedesktop icon name, a full path to a ong/svg file,
        or any name for which the application has an image handler registered.

Signals:
        onClicked:
        The signal is emmited when the button is clicked.
**/

import QtQuick 1.1
import org.kde.plasma.components 0.1 as PlasmaComponents
import "private" as Private

Item {
    id: root

    // Common Public API
    property Item tab
    property int index

    property bool pressed: mouseArea.pressed == true && mouseArea.containsMouse
    property alias text: label.text
    property alias iconSource: imageLoader.source

    property alias rightSide: rightSideItems.data

    property int iconSpacing: 4

    property int topMargin: ListView.view.buttonFrame.margins.top
    property int bottomMargin: ListView.view.buttonFrame.margins.bottom
    property int leftMargin: ListView.view.buttonFrame.margins.left
    property int rightMargin: ListView.view.buttonFrame.margins.right

    width: ListView.view.width / ListView.view.count

    implicitWidth: label.implicitWidth + (iconSource != null ? imageLoader.implicitWidth + iconSpacing : 0)
    implicitHeight: (iconSource === null
        ? label.implicitHeight
        : Math.max(label.implicitHeight, imageLoader.implicitHeight)
        ) + topMargin + bottomMargin

    opacity: enabled ? 1 : 0.6

    Private.IconLoader {
        id: imageLoader

        implicitWidth: theme.smallIconSize
        implicitHeight: implicitWidth

        anchors {
            left: parent.left
            leftMargin: leftMargin
            verticalCenter: parent.verticalCenter
        }
    }

    /*
    HACK: We use a HomerunComponents.Label for the normal text and
    a PlasmaComponents.Label for the current text.

    This is necessary for containment mode:
    It makes it possible to have custom color + shadow for normal text (to
    ensure it remains readable) and buttonTextColor + no-shadow for button text
    (because it is drawn on top of a button frame)
    */
    Label {
        id: label

        objectName: "label"

        anchors {
            top: parent.top
            left: iconSource == null ? parent.left : imageLoader.right
            leftMargin: iconSource == null ? leftMargin : iconSpacing
            right: rightSideItems.left
            bottom: parent.bottom
        }

        elide: Text.ElideRight
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter

        color: theme.textColor
        opacity: root.ListView.isCurrentItem ? 0 : 1
        Behavior on opacity { NumberAnimation { duration: 250 }}
    }

    PlasmaComponents.Label {
        id: currentLabel
        text: label.text

        anchors.fill: label

        elide: label.elide
        horizontalAlignment: label.horizontalAlignment
        verticalAlignment: label.verticalAlignment

        color: theme.buttonTextColor
        opacity: root.ListView.isCurrentItem ? 1 : 0
        Behavior on opacity { NumberAnimation { duration: 250 }}
    }

    MouseArea {
        id: mouseArea

        onClicked: {
            root.ListView.view.currentIndex = root.index;
        }

        anchors {
            top: parent.top
            left: parent.left
            right: label.right
            bottom: parent.bottom
        }
    }

    Row {
        id: rightSideItems
        anchors {
            top: parent.top
            right: parent.right
            bottom: parent.bottom
        }
    }
}
