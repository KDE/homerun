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
import org.kde.homerun.fixes 0.1 as HomerunFixes

import "KeyboardUtils.js" as KeyboardUtils

Item {
    id: main
    signal closeRequested
    property QtObject configureAction
    property bool isContainment
    property real leftMargin: 12
    property real topMargin: 12
    property real rightMargin: 12
    property real bottomMargin: 12

    property string configFileName

    property alias currentTabContent: tabGroup.currentTab

    property bool configureMode: false

    // Models
    HomerunComponents.TabModel {
        id: tabModel
        configFileName: main.configFileName
    }

    HomerunComponents.SourceRegistry {
        id: sourceRegistry
        configFileName: main.configFileName
    }

    // UI
    Component {
        id: tabContent
        TabContent {
            id: tabContentMain
            onStartedApplication: isContainment ? reset() : closeRequested()
            onSetSearchFieldRequested: searchField.text = text
        }
    }

    PlasmaComponents.TabBar {
        id: filterTabBar

        anchors {
            top: parent.top
            topMargin: main.topMargin
            left: parent.left
            leftMargin: parent.leftMargin
        }

        Repeater {
            model: tabModel
            PlasmaComponents.TabButton {
                text: model.display
                iconSource: model.decoration
                property string searchPlaceholder: model.searchPlaceholder
                property variant sources: model.sources
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
            // Workaround to make sure there is a current tab when tabModel
            // is done loading
            if (isTab(filterTabBar.currentTab)) {
                return;
            }
            filterTabBar.currentTab = firstTab();
        }

        onCurrentTabChanged: {
            if (!currentTab.tab) {
                // currentTab.tab is the tab content
                currentTab.tab = tabContent.createObject(tabGroup, {
                    "sourceRegistry": sourceRegistry,
                    "sources": currentTab.sources,
                });
            }
            // Setting currentTab does not change the tab content, so do it ourselves
            tabGroup.currentTab = currentTab.tab;
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

        function goToFirstTab() {
            currentTab = filterTabBar.firstTab();
        }

        function goToPreviousTab() {
            var lst = tabList();
            for (var idx = 1; idx < lst.length; ++idx) {
                if (lst[idx] == filterTabBar.currentTab) {
                    currentTab = lst[idx - 1];
                    return;
                }
            }
        }

        function goToNextTab() {
            var lst = tabList();
            for (var idx = 0; idx < lst.length - 1; ++idx) {
                if (lst[idx] == filterTabBar.currentTab) {
                    currentTab = lst[idx + 1];
                    return;
                }
            }
        }
    }

    // Search area
    QtExtra.QIconItem {
        anchors {
            right: searchField.left
            rightMargin: 6
            verticalCenter: searchField.verticalCenter
        }
        width: 22
        height: width
    }

    HomerunFixes.TextField {
        id: searchField

        anchors {
            right: configButton.left
            rightMargin: parent.rightMargin
            top: filterTabBar.top
            bottom: filterTabBar.bottom
        }

        width: parent.width / 4

        clearButtonShown: true
        placeholderText: filterTabBar.currentTab.searchPlaceholder

        KeyNavigation.tab: content
        KeyNavigation.backtab: content

        onTextChanged: currentTabContent.searchCriteria = text;
    }

    // Config button
    PlasmaComponents.ToolButton {
        id: configButton
        anchors {
            right: parent.right
            verticalCenter: searchField.verticalCenter
            rightMargin: parent.rightMargin
        }
        iconSource: "configure"

        property QtObject menu

        onClicked: {
            if (!menu) {
                menu = configMenuComponent.createObject(configButton);
            }
            menu.open();
        }

        Component {
            id: configMenuComponent
            PlasmaComponents.ContextMenu {
                visualParent: configButton
                PlasmaComponents.MenuItem {
                    text: configureMode ? i18n("End Configure") : i18n("Configure");
                    onClicked: {
                        configureMode = !configureMode;
                    }
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

        KeyNavigation.tab: searchField
        KeyNavigation.backtab: searchField

        onActiveFocusChanged: {
            if (activeFocus) {
                currentTabContent.forceActiveFocus();
            }
        }
    }

    // Code
    Component.onCompleted: {
        isContainment = "plasmoid" in this;
        if (isContainment) {
            // enable right click configure
            configureAction = plasmoid.action("configure");
            configureAction.enabled = true;

            // fit the containment to within the boundaries of the visible panels
            // (so no panels should be covering any information)
            // rect 0 is available screen region, rect 1 is for panels not 100% wide
            screen = plasmoid.screen
            region = plasmoid.availableScreenRegion(screen)[0]
            main.y = region.y
            main.x = region.x
            main.height = region.height
            main.width = region.width

            // Set config file only here so that when running homerunviewer with
            // a custom config file (with --config /path/to/customrc). Homerun
            // directly loads the custom config file.
            // If value of configFileName were set when it is declared, Homerun
            // would first load the default config file, then overwrite it with
            // the custom one.
            main.configFileName = "homerunrc";
        }
    }

    Connections {
        target: main
        onCurrentTabContentChanged: {
            searchField.text = currentTabContent.searchCriteria;
            currentTabContent.forceActiveFocus();
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

    function reset() {
        filterTabBar.goToFirstTab();
        filterTabBar.tabList().forEach(function(tab) {
            if (tab.tab) {
                tab.tab.reset();
            }
        });
        searchField.text = "";
    }

    Keys.onPressed: {
        var lst = [
            [Qt.ControlModifier, Qt.Key_PageUp, filterTabBar.goToPreviousTab],
            [Qt.ControlModifier, Qt.Key_PageDown, filterTabBar.goToNextTab],
            [Qt.ControlModifier, Qt.Key_F, searchField.forceActiveFocus],
        ];
        if (event.modifiers == Qt.NoModifier || event.modifiers == Qt.ShiftModifier) {
            handleTypeAheadKeyEvent(event);
        }
        KeyboardUtils.processShortcutList(lst, event);
    }

    function handleTypeAheadKeyEvent(event) {
        switch (event.key) {
        case Qt.Key_Tab:
        case Qt.Key_Escape:
            // Keys we don't want to handle as type-ahead
            return;
        case Qt.Key_Backspace:
            // Erase last char
            searchField.text = searchField.text.slice(0, -1);
            event.accepted = true;
            break;
        default:
            // Add the char to typeAhead
            if (event.text != "") {
                searchField.text += event.text;
                event.accepted = true;
            }
            break;
        }
    }
}
