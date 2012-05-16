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
    property alias pageConfigFileName: pageModel.configFileName

    Component.onCompleted: {
        plasmoid.writeConfig("favorites", "TEST", "TEST2");
        print("@@@@@: " + plasmoid.readConfig("favorites"))
    }

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

    SalComponents.PageModel { id: pageModel }

    Component {
        id: tabContent
        TabContent {}
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
                    var modelName = "RunnerModel";
                    var modelArgs;
                    var text = model.name;
                    if (text == "Dashboard") {
                        modelName = "ServiceModel";
                    } else if (text == "Applications") {
                        modelArgs = [ "services", "kill", "kget", "calculator", "audioplayercontrol" ];
                    } else if (text == "Documents") {
                        modelArgs = [ "sessions", "places", "solid" ];
                    } else if (text == "YouTube") {
                        modelArgs = "youtube";
                    } else if (text == "Bing") {
                        modelArgs = "bing";
                    }
                    tab = tabContent.createObject(tabGroup, {"modelName": modelName, "modelArgs": modelArgs});
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

        /*
        onCurrentTabChanged: {
            print("TEST" + currentTab.text)
            var text = currentTab.text
            if (text == "Categories") {
                resultsView.model = serviceModel;
                runnerModel.query = ""
            } else if (text == "Apps") {
                resultsView.model = runnerModel;
                runnerModel.runners = [ "services", "kill", "kget", "calculator", "audioplayercontrol" ]
                runnerModel.query = ""
            } else if (text == "Documents") {
                resultsView.model = runnerModel;
                runnerModel.runners = [ "sessions", "places", "solid" ]
                runnerModel.query = "places"
            } else if (text == "YouTube") {
                resultsView.model = runnerModel;
                runnerModel.runners = "youtube"
                runnerModel.query = ""
            } else if (text == "Bing") {
                resultsView.model = runnerModel;
                runnerModel.runners = "bing"
                runnerModel.query = ""
            } else if (text == "Social") {
                resultsView.model = runnerModel;
                runnerModel.query = ""
            }
            //FIXME: make it search the current lense for stuff
            //runnerModel.query = searchField.text
        }
        */
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
