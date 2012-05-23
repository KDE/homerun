/*
 *   Copyright 2011 Marco Martin <mart@kde.org>
 *   Copyright (C) 2012 by Shaun Reich <shaun.reich@blue-systems.com>
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

import Qt 4.7
import org.kde.sal.components 0.1 as SalComponents
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    id: main
    signal closeClicked
    property string configFileName

    PlasmaCore.FrameSvgItem {
        id: background
        anchors.fill: parent
        imagePath: "dialogs/background"
    }

    PlasmaComponents.ToolButton {
        anchors {
            right: main.right
            top: main.top
            rightMargin: background.margins.right
            topMargin: background.margins.top
        }
        iconSource: "window-close"
        onClicked: main.closeClicked()
    }

    SalComponents.PageModel {
        id: pageModel
        configFileName: main.configFileName
    }

    SalComponents.FavoriteModel {
        id: favoriteModel
        configFileName: main.configFileName
        function favoriteIcon(obj) {
            return "list-remove";
        }
        function triggerFavoriteAction(obj) {
            removeAt(obj.index);
        }
    }

    Component {
        id: tabContent
        TabContent {
        }
    }

    PlasmaComponents.TabBar {
        id: filterTabBar

        anchors {
            top: parent.top
            topMargin: background.margins.top
            horizontalCenter: parent.horizontalCenter
        }

        Repeater {
            model: pageModel
            PlasmaComponents.TabButton {
                text: model.name
                iconSource: model.iconName
                Component.onCompleted: {
                    tab = tabContent.createObject(tabGroup, {"sources": model.sources, "favoriteModel": favoriteModel});
                }
            }
        }

        layout.onChildrenChanged: {
            // Workaround to make sure there is a current tab when pageModel
            // is done loading
            function isTab(tab) {
                return tab && tab["iconSource"] !== undefined;
            }
            if (isTab(filterTabBar.currentTab)) {
                return;
            }
            var idx;
            for (idx = 0; idx < filterTabBar.layout.children.length; ++idx) {
                var item = filterTabBar.layout.children[idx];
                if (isTab(item)) {
                    filterTabBar.currentTab = item;
                    break;
                }
            }
        }
    }

    PlasmaComponents.TabGroup {
        id: tabGroup
        anchors {
            top: filterTabBar.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            topMargin: 12
            bottomMargin: background.margins.bottom
            leftMargin: background.margins.left
            rightMargin: background.margins.right
        }
    }
}
