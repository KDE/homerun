/*
 *   Copyright 2011 Marco Martin <mart@kde.org>
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
import org.kde.runnermodel 0.1 as RunnerModels
import org.kde.salservicemodel 0.1 as SalServiceModels
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1 as QtExtra

import "plasmapackage:/code/LayoutManager.js" as LayoutManager

Item {
    id: main

    Component.onCompleted: {

    }

    PlasmaCore.Theme {
        id: theme
    }

    PlasmaComponents.TextField {
        id: searchField

        anchors {
            top: filterTabBar.bottom
            left: filterTabBar.left
            right: filterTabBar.right
        }

        onTextChanged: {
            runnerModel.query = text;
            print("COUNT:" + runnerModel.count)
        }

        focus: true
        clearButtonShown: true
    }

    RunnerModels.RunnerModel { id: runnerModel }

    QtExtra.QIconItem {
        id: homeIcon

        anchors {
            right: filterTabBar.left
            top: filterTabBar.top
            bottom: searchField.bottom
        }

        icon: "go-home"

        width: 48
        height: 48

        MouseArea {
            anchors.fill: parent

            onClicked: {
                serviceModel.path = "/"
            }
        }
    }

    PlasmaComponents.TabBar {
        id: filterTabBar

        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }

        PlasmaComponents.TabButton { text: "All"; iconSource: ""}
        PlasmaComponents.TabButton { text: "Apps"; iconSource: "applications-other"}
        PlasmaComponents.TabButton { text: "Files"; iconSource: "folder-documents"}
        PlasmaComponents.TabButton { text: "YouTube"; iconSource: "youtube"}
        PlasmaComponents.TabButton { text: "Bing"; iconSource: "bing"}
        PlasmaComponents.TabButton { text: "Social"; iconSource: "applications-internet"}

        onCurrentTabChanged: {
            print("TEST" + currentTab.text)
            var text = currentTab.text
            if (text == "All") {

            } else if (text == "Apps") {

                runnerModel.runners = [ "services", "kill", "kget", "calculator", "audioplayercontrol" ]
            } else if (text == "Files") {
                runnerModel.runners = [ "sessions", "places", "solid" ]
            } else if (text == "YouTube") {
                runnerModel.runners = "youtube"
            } else if (text == "Bing") {
                runnerModel.runners = "bing"
            } else if (text == "Social") {

            }
            //FIXME: make it search the current lense for stuff
            //runnerModel.query = searchField.text
        }
    }

    Flow {
        anchors {
            top: searchField.bottom
            topMargin: 10
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        spacing: 2

        SalServiceModels.SalServiceModel { id: serviceModel; path: "/" }

        ResultsView {
            anchors.fill: parent
            //model: runnerModel
            model: serviceModel

            onUrlToRunChanged: {
                serviceModel.openUrl(urlToRun);
            }

            onAppIndexToRunChanged: {
                print("RUNNING APP!")
                runnerModel.run(appIndexToRun);
            }
        }
    }
}
