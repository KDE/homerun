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
    signal closeRequested

    property variant tabContentList: []

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
        onClicked: closeRequested()
    }

    SalComponents.PageModel {
        id: pageModel
    }

    SalComponents.FavoriteModel {
        id: favoriteModel

        function favoriteAction(obj) {
            return "remove";
        }
        function triggerFavoriteAction(obj) {
            removeAt(obj.index);
        }
    }

    Component {
        id: tabContent
        TabContent {
            // FIXME: If SAL is a containment mode, onResultTriggered should
            // call reset() instead of emitting closeRequested()
            onResultTriggered: closeRequested()
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
                    var lst = tabContentList;
                    lst.push(tab);
                    tabContentList = lst;
                }
            }
        }

        function isTab(tab) {
            return tab && tab["iconSource"] !== undefined;
        }

        layout.onChildrenChanged: {
            // Workaround to make sure there is a current tab when pageModel
            // is done loading
            if (isTab(filterTabBar.currentTab)) {
                return;
            }
            filterTabBar.currentTab = firstTab();
        }

        function firstTab() {
            var idx;
            for (idx = 0; idx < filterTabBar.layout.children.length; ++idx) {
                var item = filterTabBar.layout.children[idx];
                if (isTab(item)) {
                    return item;
                }
            }
            return null;
        }

        function goToFirstTab() {
            currentTab = filterTabBar.firstTab();
            // Setting currentTab does not change the tab content, so do it ourselves
            tabGroup.currentTab = currentTab.tab;
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

    function reset() {
        filterTabBar.goToFirstTab();
        tabContentList.forEach(function(content) {
            content.reset();
        });
    }
}
