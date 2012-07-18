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

import QtQuick 1.1
import org.kde.sal.components 0.1 as SalComponents
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    id: main
    signal closeRequested

    property bool embedded: false
    property real leftMargin: 12
    property real topMargin: 12
    property real rightMargin: 12
    property real bottomMargin: 12

    property variant tabContentList: []

    PlasmaComponents.ToolButton {
        anchors {
            right: main.right
            top: main.top
            rightMargin: main.rightMargin
            topMargin: main.topMargin
        }

        visible: embedded

        iconSource: "window-close"
        onClicked: closeRequested()
    }

    SalComponents.PageModel {
        id: pageModel
    }

    SalComponents.FavoriteAppsModel {
        id: favoriteAppsModel
        property string name: "Favorite Applications"
    }

    SalComponents.FavoritePlacesModel {
        id: favoritePlacesModel
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
            topMargin: main.topMargin
            horizontalCenter: parent.horizontalCenter
        }

        Repeater {
            model: pageModel
            PlasmaComponents.TabButton {
                text: model.name
                iconSource: model.iconName
                Component.onCompleted: {
                    var favoriteModels = new Object();
                    favoriteModels[favoriteAppsModel.favoritePrefix] = favoriteAppsModel;
                    favoriteModels[favoritePlacesModel.favoritePrefix] = favoritePlacesModel;

                    // This should not be "var tab": we set the "tab" property of the TabButton
                    tab = tabContent.createObject(tabGroup, {"sources": model.sources, "favoriteModels": favoriteModels});
                    var lst = tabContentList;
                    if (lst.length == 0) {
                        tab.forceActiveFocus();
                    }
                    lst.push(tab);
                    tabContentList = lst;
                }
            }
        }

        Connections {
            target: tabGroup
            onCurrentTabChanged: {
                tabGroup.currentTab.forceActiveFocus();
            }
        }

        /**
         * Use duck-typing to determine if an item is a tab and not a Repeater
         * or something else.
         */
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
            for (var idx = 0; idx < filterTabBar.layout.children.length; ++idx) {
                var item = filterTabBar.layout.children[idx];
                if (isTab(item)) {
                    return item;
                }
            }
            return null;
        }

        /**
         * Return a list of all children which are actually tabs
         */
        function tabList() {
            var lst = new Array();
            for (var idx = 0; idx < filterTabBar.layout.children.length; ++idx) {
                var item = filterTabBar.layout.children[idx];
                if (isTab(item)) {
                    lst.push(item);
                }
            }
            return lst;
        }

        function setCurrentTab(tab) {
            currentTab = tab;
            // Setting currentTab does not change the tab content, so do it ourselves
            tabGroup.currentTab = currentTab.tab;
        }

        function goToFirstTab() {
            setCurrentTab(filterTabBar.firstTab());
        }

        function goToPreviousTab() {
            var lst = tabList();
            for (var idx = 1; idx < lst.length; ++idx) {
                if (lst[idx] == filterTabBar.currentTab) {
                    setCurrentTab(lst[idx - 1]);
                    return;
                }
            }
        }

        function goToNextTab() {
            var lst = tabList();
            for (var idx = 0; idx < lst.length - 1; ++idx) {
                if (lst[idx] == filterTabBar.currentTab) {
                    setCurrentTab(lst[idx + 1]);
                    return;
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
            bottomMargin: main.bottomMargin
            leftMargin: main.leftMargin
            rightMargin: main.rightMargin
        }
    }

    function reset() {
        filterTabBar.goToFirstTab();
        tabContentList.forEach(function(content) {
            content.reset();
        });
    }

    Keys.onPressed: {
        var lst = [
            [Qt.ControlModifier, Qt.Key_PageUp, filterTabBar.goToPreviousTab],
            [Qt.ControlModifier, Qt.Key_PageDown, filterTabBar.goToNextTab],
        ];
        event.accepted = lst.some(function(x) {
            if (event.modifiers == x[0] && event.key == x[1]) {
                x[2]();
            }
        });
    }
}
