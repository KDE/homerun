/*
 *   Copyright (C) 2013 by Eike Hein <hein@kde.org>
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

Item {
    property bool vertical: (plasmoid.formFactor == Vertical)

    PlasmaCore.IconItem {
        id: buttonIcon

        anchors.fill: parent

        visible: !(main.useCustomButtonImage && main.buttonImage)

        source: main.icon
        active: mouseArea.containsMouse

        onWidthChanged: {
            if (vertical && visible) {
                plasmoid.setMinimumSize(theme.smallIconSize, parent.width);
            }
        }

        onHeightChanged: {
            if (!vertical && visible) {
                plasmoid.setMinimumSize(parent.height, theme.smallIconSize);
            }
        }

        onVisibleChanged: {
            if (visible) {
                if (vertical) {
                    plasmoid.setMinimumSize(theme.smallIconSize, parent.width);
                } else {
                    plasmoid.setMinimumSize(parent.height, theme.smallIconSize);
                }
            }
        }
    }

    Image {
        id: buttonImage

        width: vertical ? parent.width : undefined
        height: vertical ? undefined : parent.height

        onPaintedWidthChanged: {
            if (!vertical && visible) {
                plasmoid.setMinimumSize(paintedWidth, theme.smallIconSize);
            }
        }

        onPaintedHeightChanged: {
            if (vertical && visible) {
                plasmoid.setMinimumSize(theme.smallIconSize, paintedHeight);
            }
        }

        onVisibleChanged: {
            if (visible) {
                if (vertical) {
                    plasmoid.setMinimumSize(theme.smallIconSize, paintedHeight);
                } else {
                    plasmoid.setMinimumSize(paintedWidth, theme.smallIconSize);
                }
            }
        }

        visible: main.useCustomButtonImage && main.buttonImage
        source: main.buttonImage
        fillMode: Image.PreserveAspectFit
        smooth: true
    }

    MouseArea
    {
        id: mouseArea

        anchors.fill: parent

        hoverEnabled: true

        onClicked: {
            if (plasmoid.popupShowing) {
                plasmoid.hidePopup();
            } else {
                plasmoid.showPopup();
            }
        }
    }
}
