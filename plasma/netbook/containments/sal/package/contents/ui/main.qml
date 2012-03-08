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
import org.kde.runnermodel 0.1 as RunnerModels
import org.kde.salservicemodel 0.1 as SalServiceModels
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1 as QtExtra

import "plasmapackage:/code/LayoutManager.js" as LayoutManager

Item {
    id: main

    width: 2000
    height: 3000

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
            if (text == "") {
                // it's null, probably makes sense to switch back to the category model
                resultsView.model = serviceModel;
            } else {
                resultsView.model = runnerModel;
                runnerModel.query = text;
                print("COUNT:" + runnerModel.count)
            }
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

            hoverEnabled: true

            onClicked: {
                serviceModel.path = "/"
                resultsView.model = serviceModel
            }

            //FIXME: add an svg highlight to it on mouse over. that'll look much better me thinks
            onEntered: {
                print("ENTERED!");
                parent.width = 55;
                parent.height = 55;
            }

            onExited: {
                parent.width = 48;
                parent.height = 48;
            }
        }
    }

    PlasmaComponents.TabBar {
        id: filterTabBar

        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }

        PlasmaComponents.TabButton { text: "Categories"; iconSource: ""}
        PlasmaComponents.TabButton { text: "Apps"; iconSource: "applications-other"}
        PlasmaComponents.TabButton { text: "Files"; iconSource: "folder-documents"}
        PlasmaComponents.TabButton { text: "YouTube"; iconSource: "youtube"}
        PlasmaComponents.TabButton { text: "Bing"; iconSource: "bing"}
        PlasmaComponents.TabButton { text: "Social"; iconSource: "applications-internet"}

        onCurrentTabChanged: {
            plasmoid.writeConfig("TEST", "TEST")
            print("TEST" + currentTab.text)
            var text = currentTab.text
            if (text == "Categories") {
                resultsView.model = serviceModel;
                runnerModel.query = ""
            } else if (text == "Apps") {
                resultsView.model = runnerModel;
                runnerModel.runners = [ "services", "kill", "kget", "calculator", "audioplayercontrol" ]
                runnerModel.query = ""
            } else if (text == "Files") {
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
    }

    SalServiceModels.SalServiceModel { id: serviceModel; path: "/" }

    ResultsView {
        id: resultsView
        anchors {
            top: searchField.bottom
            bottom: favoritesView.top
            left: parent.left
            right: parent.right
        }

        model: serviceModel

        onUrlToRunChanged: {
            serviceModel.openUrl(urlToRun);
        }

        onAppIndexToRunChanged: {
            if (resultsView.model == runnerModel) {
                print("RUNNING APP!")
                runnerModel.run(appIndexToRun);
            }

            print( "%%%%%%%%%%%%%%%%%%% APPENDING ICON: " + currentIcon);
            favoritesModel.append( {"text": currentString, "icon": currentIcon });
        }
    }

    ListModel {
        id: favoritesModel

        ListElement { text: "TEST1"; icon: "bookmarks"}
        ListElement { text: "TEST2"; icon: "bookmarks"}
        ListElement { text: "TEST3"; icon: "bookmarks"}
        ListElement { text: "TEST4"; icon: "bookmarks"}
        ListElement { text: "TEST5"; icon: "bookmarks"}
    }

    GridView {
        id: favoritesView

        anchors {
            left: parent.left;
            right: parent.right
            bottom: parent.bottom
        }

        clip: true
        interactive: false
        //FIXME: use anchoring
        height: 90

        model: favoritesModel
        delegate: Favorite {
            id: favorite;
            currentText: model.text;
            currentIcon: model.icon
        }
    }
}
