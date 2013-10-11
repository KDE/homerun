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

import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.extras 0.1 as PlasmaExtras

import org.kde.homerun.components 0.1 as HomerunComponents

PlasmaExtras.ScrollArea {
    id: itemList

    property QtObject model
    property alias contentHeight: listView.contentHeight
    property alias currentIndex: listView.currentIndex
    property alias currentItem: listView.currentItem
    property bool expandable: false
    property int eligibleWidth

    ListView {
        id: listView

        focus: parent.focus

        onFocusChanged: {
            currentIndex = focus ? 0 : -1;
        }

        currentIndex: -1

        model: itemList.model

        onModelChanged: {
            currentIndex = plasmoid.popupShowing && expandable ? 0 : -1;
        }

        delegate: ItemListDelegate {}

        highlight: PlasmaComponents.Highlight {}
        highlightFollowsCurrentItem: true
        highlightMoveDuration: 250
    }

    Keys.onReturnPressed: {
        if (!expandable) {
            model.trigger(currentIndex, "", null);
        }
    }

    Keys.onEnterPressed: {
            if (!expandable) {
            model.trigger(currentIndex, "", null);
        }
    }
}
