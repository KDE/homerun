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

Item {
    id: main
    signal closeRequested
    signal applicationLaunched(string application)
    property QtObject configureAction
    property bool isContainment

    property string configFileName

    property alias currentTabContent: tabGroup.currentTab

    property bool configureMode: false
    property bool showActionListOverlay: globalSettings.showActionListOverlay
    property bool showDesktopToolBox: globalSettings.showDesktopToolBox

    // Internal
    property real outerPadding: 12
    property real innerPadding: 6 // padding between header items

    HomerunComponents.GlobalSettings {
        id: globalSettings
        configFileName: main.configFileName
    }

    // Models
    HomerunComponents.TabModel {
        id: tabModel
        configFileName: main.configFileName
        sourceRegistry: sourceRegistry
    }

    HomerunComponents.SourceRegistry {
        id: sourceRegistry
        configFileName: main.configFileName
    }

    HomerunFixes.ToolBoxToggle {
        id: toolBoxToggle
        visible: showDesktopToolBox
    }

    // Components
    Component {
        id: tabContentComponent
        TabContent {
            id: tabContentMain
            property Item tabButton
            rootItem: main
            tabIconSource: tabButton.iconSource
            tabText: tabButton.realText
            configureMode: main.configureMode
            showActionListOverlay: main.showActionListOverlay
            onCloseRequested: isContainment ? reset() : main.closeRequested()
            onSetSearchFieldRequested: searchField.text = text
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

    // Actions (used for keyboard shortcuts)
    HomerunComponents.ActionManager {
        id: actionManager
        configFileName: main.configFileName
        HomerunComponents.Action {
            objectName: "go-to-previous-tab"
            text: i18n("Go to previous tab")
            keys: ["Ctrl+PgUp"]
            onTriggered: tabBar.decrementCurrentIndex()
        }
        HomerunComponents.Action {
            objectName: "go-to-next-tab"
            text: i18n("Go to next tab")
            keys: ["Ctrl+PgDown"]
            onTriggered: tabBar.incrementCurrentIndex()
        }
        HomerunComponents.Action {
            objectName: "search"
            text: i18n("Focus search field")
            keys: ["Ctrl+F", "/"]
            onTriggered: searchField.forceActiveFocus()
        }
        HomerunComponents.Action {
            objectName: "go-back"
            text: i18n("Go back")
            keys: ["Alt+Left"]
            onTriggered: currentTabContent.goBack()
        }
        HomerunComponents.Action {
            objectName: "go-forward"
            text: i18n("Go forward")
            keys: ["Alt+Right"]
            onTriggered: currentTabContent.goBack()
        }
        HomerunComponents.Action {
            objectName: "go-up"
            text: i18n("Go up")
            keys: ["Alt+up"]
            onTriggered: currentTabContent.goUp()
        }
    }

    // UI
    HomerunComponents.TabBar {
        id: tabBar

        property bool selectNewTab: false

        anchors {
            top: parent.top
            topMargin: main.outerPadding
            left: parent.left
            leftMargin: main.outerPadding
            right: configureMode ? addTabButton.left : searchField.left
            rightMargin: configureMode ? 0 : innerPadding
        }

        model: tabModel
        delegate: HomerunComponents.TabButton {
            id: tabButtonMain
            property string realText: model.display
            text: model.display || i18nc("Used for tabs which have no name", "<Untitled>")
            iconSource: model.decoration
            property QtObject tabSourceModel: model.sourceModel
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
            var hadFocus = !searchField.activeFocus;
            tabGroup.currentTab = currentItem.tab;
            if (hadFocus) {
                tabGroup.currentTab.forceActiveFocus();
            }
        }

        function createTabContent(tabButton) {
            tabButton.tab = tabContentComponent.createObject(tabGroup, {
                tabSourceModel: tabButton.tabSourceModel,
                sourceRegistry: sourceRegistry,
                tabButton: tabButton,
            });
        }

        onCountChanged: {
            if (selectNewTab) {
                selectNewTab = false;
                currentIndex = count - 1;
            }
        }
    }

    // Add tab button
    PlasmaComponents.ToolButton {
        id: addTabButton
        anchors {
            right: endConfigureButton.left
            top: tabBar.top
            bottom: tabBar.bottom
            rightMargin: innerPadding
        }
        width: height + 1 // +1 to workaround ToolButton using a round frame if width == height
        opacity: configureMode ? 1 : 0

        flat: false
        iconSource: "list-add"
        onClicked: {
            tabBar.selectNewTab = true;
            tabModel.appendRow();
        }
    }

    // End config button
    PlasmaComponents.ToolButton {
        id: endConfigureButton
        anchors {
            right: configButton.left
            top: tabBar.top
            bottom: tabBar.bottom
            rightMargin: innerPadding
        }
        opacity: configureMode ? 1 : 0

        flat: false
        text: i18nc("Button to leave configure mode", "Done")
        onClicked: configureMode = false;
    }


    // Search area
    HomerunFixes.TextField {
        id: searchField

        anchors {
            right: configButton.left
            top: tabBar.top
            bottom: tabBar.bottom
            rightMargin: innerPadding
        }

        width: parent.width / 4
        opacity: configureMode ? 0 : 1

        focus: true

        clearButtonShown: true
        placeholderText: i18n("Search in current tab...")

        KeyNavigation.tab: content
        KeyNavigation.backtab: content
        KeyNavigation.down: content

        onTextChanged: currentTabContent.searchCriteria = text;

        onAccepted: currentTabContent.triggerFirstItem()
    }

    // Config button
    PlasmaComponents.ToolButton {
        id: configButton
        anchors {
            right: parent.right
            top: tabBar.top
            bottom: tabBar.bottom
            rightMargin: main.outerPadding
        }
        iconSource: "applications-system"

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
                    text: configureMode ? i18n("End Configure Tabs") : i18n("Configure Tabs");
                    onClicked: {
                        configureMode = !configureMode;
                        if (configureMode) {
                            currentTabContent.reset();
                        }
                    }
                }
                PlasmaComponents.MenuItem {
                    text: i18n("Configure Shortcuts...")
                    onClicked: actionManager.configure()
                }
                PlasmaComponents.MenuItem {
                    text: i18n("Show Action List Buttons");
                    checkable: true
                    checked: showActionListOverlay
                    onCheckedChanged: {
                        globalSettings.showActionListOverlay = checked;
                    }
                }
                PlasmaComponents.MenuItem {
                    visible: isContainment
                    text: i18n("Show Desktop Tool Box");
                    checkable: true
                    checked: showDesktopToolBox
                    onCheckedChanged: {
                        globalSettings.showDesktopToolBox = checked;
                    }
                }
                PlasmaComponents.MenuItem {
                    separator: true
                }
                PlasmaComponents.MenuItem {
                    text: i18n("Restore Tabs to Default");
                    onClicked: {
                        var ret = messageBox.warningContinueCancel(
                            i18n("Are you sure you want to restore tabs?\nRestoring tabs will reset your configuration to the default Homerun tabs.\nYour lists of favorite applications and places will remain untouched."),
                            i18n("Restore Tabs"),
                            i18n("Restore Tabs"));

                        if (ret == HomerunComponents.MessageBox.Continue) {
                            tabModel.resetConfig();
                        }
                    }
                }
                PlasmaComponents.MenuItem {
                    separator: true
                }
                PlasmaComponents.MenuItem {
                    // We do not use helpMenuActions.text(HomerunComponents.HelpMenuActions.AboutApplication)
                    // because it returns Plasma when running as a containment
                    text: i18n("About %1", "Homerun")
                    onClicked: helpMenuActions.trigger(HomerunComponents.HelpMenuActions.AboutApplication)
                }
                PlasmaComponents.MenuItem {
                    text: helpMenuActions.text(HomerunComponents.HelpMenuActions.ReportBug)
                    onClicked: helpMenuActions.trigger(HomerunComponents.HelpMenuActions.ReportBug)
                }
            }
        }

        HomerunComponents.HelpMenuActions {
            id: helpMenuActions
        }
        HomerunComponents.MessageBox {
            id: messageBox
        }
    }

    // Main content
    Item {
        id: content
        anchors {
            top: tabBar.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            topMargin: main.outerPadding
        }

        HomerunComponents.TabGroup {
            id: tabGroup
            anchors.fill: parent
        }

        KeyNavigation.tab: searchField
        KeyNavigation.backtab: searchField
        KeyNavigation.up: searchField

        onActiveFocusChanged: {
            if (activeFocus) {
                currentTabContent.forceActiveFocus();
            }
        }
    }

    function resizeToFitScreen() {
        // fit the containment to within the boundaries of the visible panels
        // (so no panels should be covering any information)
        // rect 0 is available screen region, rect 1 is for panels not 100% wide
        var screen = plasmoid.screen;
        var sourceRegion = plasmoid.availableScreenRegion(screen)[1];
        if (sourceRegion === undefined) {
            sourceRegion = plasmoid.availableScreenRegion(screen)[0];
        }

        main.y = sourceRegion.y;
        main.x = sourceRegion.x;
        main.height = sourceRegion.height;
        main.width = sourceRegion.width;
    }

    // Code
    Component.onCompleted: {
        isContainment = "plasmoid" in this;
        if (isContainment) {
            // enable right click configure
            configureAction = plasmoid.action("configure");
            configureAction.enabled = true;

            plasmoid.availableScreenRegionChanged.connect(resizeToFitScreen);
            resizeToFitScreen();

            toolBoxToggle.toolBox = plasmoid.toolBox;

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
        }
    }

    function reset() {
        tabBar.currentIndex = 0;
        for (idx = 0; idx < tabGroup.data.length; ++idx) {
            var tabContent = tabGroup.data[idx];
            if (tabContent && tabContent.reset) {
                tabContent.reset();
            }
        }
        configureMode = false;
        searchField.text = "";
        searchField.forceActiveFocus();
    }
}
