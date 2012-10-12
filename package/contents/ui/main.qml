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
            property Item tabButton
            tabIconSource: tabButton.iconSource
            tabText: tabButton.realText
            configureMode: main.configureMode
            onCloseRequested: isContainment ? reset() : main.closeRequested()
            onSetSearchFieldRequested: searchField.text = text
            onSourcesUpdated: {
                tabModel.setSourcesForRow(tabButton.index, sources);
            }
            onTabTextChanged: {
                if (configureMode) {
                    tabModel.setDataForRow(tabButton.index, "display", tabText);
                }
            }
            onTabIconSourceChanged: {
                if (configureMode) {
                    tabModel.setDataForRow(tabButton.index, "decoration", tabIconSource);
                }
            }
        }
    }

    HomerunComponents.TabBar {
        id: filterTabBar

        anchors {
            top: parent.top
            topMargin: main.topMargin
            left: parent.left
            leftMargin: parent.leftMargin
            right: searchField.left
            rightMargin: 6 + (configureMode ? addTabButton.width : 0)
        }

        model: tabModel

        delegate: HomerunComponents.TabButton {
            id: tabButtonMain
            property string realText: model.display
            text: model.display || i18nc("Used for tabs which have no name", "<Untitled>")
            iconSource: model.decoration
            property string searchPlaceholder: model.searchPlaceholder
            property variant sources: model.sources
            index: model.index

            rightSide: [
                TabSideButton {
                    opacity: configureMode ? 1 : 0
                    iconSource: "go-previous"
                    enabled: index > 0
                    onClicked: tabModel.moveRow(index, index - 1)
                },
                TabSideButton {
                    opacity: configureMode ? 1 : 0
                    iconSource: "go-next"
                    enabled: index < tabButtonMain.ListView.view.count - 1
                    onClicked: tabModel.moveRow(index, index + 1)
                },
                TabSideButton {
                    opacity: configureMode ? 1 : 0
                    iconSource: "list-remove"
                    onClicked: tabModel.removeRow(index)
                }
            ]
        }

        onCurrentItemChanged: {
            if (!currentItem.tab) {
                createTabContent(currentItem);
            }
            // Setting currentTab does not change the tab content, so do it ourselves
            tabGroup.currentTab = currentItem.tab;
        }

        function createTabContent(tabButton) {
            tabButton.tab = tabContent.createObject(tabGroup, {
                sources: tabButton.sources,
                sourceRegistry: sourceRegistry,
                tabButton: tabButton,
            });
        }
    }

    // Add tab button
    PlasmaComponents.ToolButton {
        id: addTabButton
        anchors {
            left: filterTabBar.right
            top: filterTabBar.top
            bottom: filterTabBar.bottom
        }
        opacity: configureMode ? 1 : 0
        iconSource: "list-add"
        onClicked: tabModel.appendRow();
    }

    // Search area
    HomerunFixes.TextField {
        id: searchField

        anchors {
            right: configButton.left
            top: filterTabBar.top
            bottom: filterTabBar.bottom
        }

        width: parent.width / 4

        clearButtonShown: true
        placeholderText: filterTabBar.currentItem.searchPlaceholder

        KeyNavigation.tab: content
        KeyNavigation.backtab: content

        onTextChanged: currentTabContent.searchCriteria = text;
    }

    // Config button
    PlasmaComponents.ToolButton {
        id: configButton
        anchors {
            right: parent.right
            top: filterTabBar.top
            bottom: filterTabBar.bottom
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
                    onClicked: configureMode = !configureMode;
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

        HomerunComponents.TabGroup {
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

    function reset() {
        filterTabBar.currentIndex = 0;
        for (idx = 0; idx < tabGroup.data.length; ++idx) {
            var tabContent = tabGroup.data[idx];
            if (tabContent && tabContent.reset) {
                tabContent.reset();
            }
        }
        searchField.text = "";
    }

    Keys.onPressed: {
        var lst = [
            [Qt.ControlModifier, Qt.Key_PageUp, filterTabBar.decrementCurrentIndex],
            [Qt.ControlModifier, Qt.Key_PageDown, filterTabBar.incrementCurrentIndex],
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
