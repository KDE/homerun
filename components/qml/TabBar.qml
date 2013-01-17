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
 *
 *- End of KDE Plasma Components TabBar.qml copyright header ------------
 */

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1

Item {
    id: root

    height: /*frame.margins.top
        +*/ buttonFrame.margins.top
        + theme.smallIconSize
        + buttonFrame.margins.bottom
        + 6
        //+ frame.margins.bottom

    property alias model: listView.model
    property alias delegate: listView.delegate
    property alias currentIndex: listView.currentIndex
    property alias currentItem: listView.currentItem
    property alias count: listView.count

    function incrementCurrentIndex() {
        listView.incrementCurrentIndex();
    }

    function decrementCurrentIndex() {
        listView.decrementCurrentIndex();
    }

    property alias buttonFrame: listView.buttonFrame

    property string outerImagePath: "widgets/button"
    property string outerPrefix: "pressed"

    PlasmaCore.FrameSvgItem {
        id: leftFrame

        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
        }
        opacity: listView.currentIndex > 0 ? 0.6 : 0
        width: listView.currentItem.x
        imagePath: outerImagePath
        prefix: outerPrefix
        enabledBorders: "BottomBorder|RightBorder"
        Behavior on width { NumberAnimation {duration: 100}}
    }

    PlasmaCore.FrameSvgItem {
        id: rightFrame

        anchors {
            top: parent.top
            bottom: parent.bottom
            right: parent.right
        }
        width: listView.width - listView.currentItem.x - listView.currentItem.width
        opacity: (listView.currentIndex < listView.count - 1) ? 0.6 : 0
        imagePath: outerImagePath
        prefix: outerPrefix
        enabledBorders: "LeftBorder|BottomBorder"
        Behavior on width { NumberAnimation {duration: 100}}
    }

    Item {
        anchors.fill: parent
        /*
        PlasmaCore.FrameSvgItem {
            id: frame

            anchors.fill: parent
            imagePath: "translucent/dialogs/background"
            //imagePath: "widgets/frame"
            //prefix: "pressed"
        }*/

        ListView {
            id: listView

            // Hack: make it possible for other elements to know the margins
            // of the highlight item before it is created
            property Item buttonFrame: PlasmaCore.FrameSvgItem {
                imagePath: "widgets/button"
                prefix: "normal"
                visible: false
            }

            anchors {
                left: parent.left
                top: parent.top
                right: parent.right
                bottom: parent.bottom
                /*leftMargin: frame.margins.left
                topMargin: frame.margins.top
                rightMargin: frame.margins.right
                bottomMargin: frame.margins.bottom*/
            }

            orientation: ListView.Horizontal

            /*
            highlightMoveDuration: 250
            highlightResizeDuration: 250

            highlight: PlasmaCore.FrameSvgItem {
                imagePath: "widgets/button"
                prefix: "normal"
                height: parent ? parent.height : 0
            }
            */

            currentIndex: 0
        }
    }
}
