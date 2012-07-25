/*
 *   Copyright 2012 Aurélien Gâteau <agateau@kde.org>
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
import org.kde.sal.fixes 0.1 as SalFixes
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1 as QtExtra

import "KeyboardUtils.js" as KeyboardUtils

import "TabContentInternal.js" as TabContentInternal

Item {
    id: main

    property variant favoriteModels
    property variant sources
    property string typeAhead

    property bool searchable: searchModels.length > 0
    property string searchCriteria

    signal startedApplication
    signal updateTabOrderRequested

    // Internal
    property variant browseModels: []
    property variant searchModels: []

    property bool componentCompleted: false

    property bool isSearching: searchCriteria.length == 0
    onIsSearchingChanged: {
        if (componentCompleted) {
            createPage(isSearching ? browseModels : searchModels);
        }
    }

    SalComponents.SharedConfig {
        id: config
        name: "salrc"
    }

    // Models
    Component {
        id: serviceModelComponent
        SalComponents.SalServiceModel {
            path: "/"
            property string name: "Applications"
            installer: config.readEntry("PackageManagement", "categoryInstaller")

            onOpenSourceRequested: {
                openSource(source);
            }
        }
    }

    Component {
        id: powerModelComponent
        SalComponents.PowerModel {
            property string name: "Power"
        }
    }

    Component {
        id: sessionModelComponent
        SalComponents.SessionModel {
            property string name: "Session"
        }
    }

    Component {
        id: runnerModelComponent
        SalComponents.SalRunnerModel {
            query: searchCriteria
        }
    }

    Component {
        id: placesModelComponent
        SalComponents.PlacesModel {
            property string name: "Favorite Places"
            rootModel: main.favoriteModels["place"]

            onOpenSourceRequested: {
                openSource(source);
            }
        }
    }

    // UI components
    Component {
        id: multiResultsViewComponent
        Repeater {
            id: repeater
            property variant favoriteModels
            delegate: ResultsView {
                width: parent.width
                model: repeater.model.modelForRow(index) // Here "index" is the current row number within the repeater
                typeAhead: main.typeAhead
                favoriteModels: repeater.favoriteModels
                onIndexClicked: {
                    // Here "index" is the row number clicked inside the ResultsView
                    handleTriggerResult(model.trigger(index));
                }
            }

            /*
            onItemAdded: updateTabOrderRequested()
            onItemRemoved: updateTabOrderRequested()
            */
        }
    }

    Component {
        id: resultsViewComponent
        ResultsView {
            width: parent.width
            typeAhead: main.typeAhead

            onIndexClicked: {
                handleTriggerResult(model.trigger(index));
            }
        }
    }

    Component {
        id: pageComponent

        Item {
            property alias viewContainer: column
            anchors.fill: parent

            Flickable {
                id: flickable
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                    left: parent.left
                    right: scrollBar.left
                }
                contentWidth: width
                contentHeight: column.height
                clip: true
                Column {
                    id: column
                    width: parent.width
                }
            }

            PlasmaComponents.ScrollBar {
                id: scrollBar
                flickableItem: flickable
                anchors {
                    right: parent.right
                    top: parent.top
                    bottom: parent.bottom
                }
            }

            Behavior on opacity {
                NumberAnimation {
                    duration: 200
                }
            }
        }
    }

    // Ui
    PlasmaComponents.ToolButton {
        id: backButton
        anchors {
            left: parent.left
            top: parent.top
        }
        iconSource: "go-previous"
        onClicked: {
            TabContentInternal.goBack();
        }
        z: 1000 // FIXME: Ugly
    }
    PlasmaComponents.ToolButton {
        id: forwardButton
        anchors {
            left: backButton.right
            top: parent.top
        }
        iconSource: "go-next"
        onClicked: {
            TabContentInternal.goForward();
        }
        z: 1000 // FIXME: Ugly
    }

    PlasmaComponents.Label {
        id: typeAheadLabel
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top
        }
        text: typeAhead + "|"
        font.pointSize: 14
        opacity: typeAhead == "" ? 0 : 0.4
    }

    // Scripting
    Component.onCompleted: {
        createModels();
        createPage(browseModels);
        componentCompleted = true;
    }

    function handleTriggerResult(result) {
        main.typeAhead = "";
        if (result) {
            startedApplication();
            return;
        }
    }

    function openSource(source) {
        var out = createModelForSource(source);
        var models = [out[0]];
        createPage(models);
    }

    Keys.onPressed: {
        switch (event.key) {
        case Qt.Key_Backspace:
            typeAhead = typeAhead.slice(0, -1);
            event.accepted = true;
            break;
        case Qt.Key_Tab:
            break;
        default:
            if (event.text != "") {
                typeAhead += event.text;
                event.accepted = true;
            }
            break;
        }
    }

    function createModels() {
        var searchLst = new Array();
        var browseLst = new Array();
        sources.forEach(function(source) {
            var out = createModelForSource(source);
            var model = out[0];
            var isSearchModel = out[1];
            if (isSearchModel) {
                searchLst.push(model);
            } else {
                browseLst.push(model);
            }
        });
        // We can't mutate arrays which are defined outside of a JS function,
        // so we replace them instead
        searchModels = searchLst;
        browseModels = browseLst;
    }

    function createModelForSource(source) {
        var idx = source.indexOf(":");
        var modelName;
        var modelArgs;
        if (idx > 0) {
            modelName = source.slice(0, idx);
            modelArgs = source.slice(idx + 1);
        } else {
            modelName = source;
        }
        var model;
        if (modelName == "ServiceModel") {
            model = serviceModelComponent.createObject(main);
        } else if (modelName == "PlacesModel") {
            model = placesModelComponent.createObject(main);
        } else if (modelName == "FavoriteAppsModel") {
            model = main.favoriteModels["app"];
        } else if (modelName == "PowerModel") {
            model = powerModelComponent.createObject(main);
        } else if (modelName == "SessionModel") {
            model = sessionModelComponent.createObject(main);
        } else if (modelName == "RunnerModel") {
            model = runnerModelComponent.createObject(main);
        } else {
            console.log("Error: unknown model type: " + modelName);
            return;
        }
        var isSearchModel = modelName == "RunnerModel"; // FIXME
        model.objectName = source;

        if (modelArgs) {
            if ("arguments" in model) {
                model.arguments = modelArgs;
            } else {
                console.log("Error: trying to set arguments on model " + model + ", which does not support arguments");
            }
        }
        return [model, isSearchModel];
    }

    function createPage(models) {
        var page = pageComponent.createObject(main);
        models.forEach(function(model) {
            var component = "modelForRow" in model ? multiResultsViewComponent : resultsViewComponent;
            component.createObject(page.viewContainer, {"model": model, "favoriteModels": favoriteModels});
        });

        TabContentInternal.addPage(page);
        TabContentInternal.goTo(TabContentInternal.pages.length - 1);
        //updateTabOrderRequested();
    }

    function reset() {
        searchCriteria = "";
        browseModels.forEach(function(model) {
            if ("path" in model) {
                model.path = "/";
            }
        });
    }
}
