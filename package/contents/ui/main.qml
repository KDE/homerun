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
import org.kde.homerun.components 0.1 as HomerunComponents
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1 as QtExtra

import "KeyboardUtils.js" as KeyboardUtils

Item {
    id: main
    signal closeRequested

    property bool embedded: false
    property real leftMargin: 12
    property real topMargin: 12
    property real rightMargin: 12
    property real bottomMargin: 12

    property variant tabContentList: []
    property alias currentTabContent: tabGroup.currentTab

    // Models
    HomerunComponents.PageModel {
        id: pageModel
    }

    HomerunComponents.FavoriteAppsModel {
        id: favoriteAppsModel
    }

    HomerunComponents.FavoritePlacesModel {
        id: favoritePlacesModel
    }

    // UI
    Component {
        id: tabContent
        TabContent {
            id: tabContentMain
            onStartedApplication: embedded ? closeRequested() : reset()
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
                    // "tab" is a property of TabButton, that is why it is not declared with "var"
                    tab = createTabContent(tabGroup, model.sources);
                    var lst = tabContentList;
                    lst.push(tab);
                    tabContentList = lst;
                }
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

    // Main content
    Item {
        id: content
        anchors {
            top: filterTabBar.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            topMargin: 4
            bottomMargin: main.bottomMargin
            leftMargin: main.leftMargin
            rightMargin: main.rightMargin
        }

        PlasmaComponents.TabGroup {
            id: tabGroup
            anchors.fill: parent
        }

        /*
        TabContent {
            id: searchTabContent
            anchors.fill: parent
            opacity: searchField.searching ? 1 : 0
            sources: ["RunnerModel"]
            favoriteModels: createFavoriteModelsObject();
            searchCriteria: searchField.text
        }

        KeyNavigation.tab: searchField
        KeyNavigation.backtab: searchField

        onActiveFocusChanged: {
            if (!activeFocus) {
                return;
            }
            if (searchField.searching) {
                searchTabContent.forceActiveFocus();
            } else {
                currentTabContent.forceActiveFocus();
            }
        }
        */
        onActiveFocusChanged: {
            if (activeFocus) {
                currentTabContent.forceActiveFocus();
            }
        }
    }

    // Code
    Connections {
        target: main
        onCurrentTabContentChanged: {
            console.log("main.onCurrentTabContentChanged");
            currentTabContent.forceActiveFocus();
            //focusFirstView();
        }
    }

    function focusFirstView() {
        var page = currentTabContent.currentPage;
        if (!page) {
            return;
        }
        var firstView = page.getFirstView();
        if (!firstView) {
            return;
        }
        firstView.forceActiveFocus();
    }

    function createFavoriteModelsObject() {
        var favoriteModels = new Object();
        favoriteModels[favoriteAppsModel.favoritePrefix] = favoriteAppsModel;
        favoriteModels[favoritePlacesModel.favoritePrefix] = favoritePlacesModel;
        return favoriteModels;
    }

    function createTabContent(parent, sources) {
        var favoriteModels = createFavoriteModelsObject();
        return tabContent.createObject(parent, {"sources": sources, "favoriteModels": favoriteModels});
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
        KeyboardUtils.processShortcutList(lst, event);
    }
}
