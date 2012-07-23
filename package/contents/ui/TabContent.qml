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

Item {
    id: main

    property variant favoriteModels
    property variant sources

    property bool searchable: searchModels.length > 0
    property string searchCriteria

    signal resultTriggered
    signal updateTabOrderRequested

    // Internal
    property variant browseModels: []
    property variant searchModels: []
    property Item page

    property string typeAhead

    property bool isSearching: searchCriteria.length == 0
    onIsSearchingChanged: {
        createPage(isSearching ? browseModels : searchModels);
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
                    main.typeAhead = "";
                    // Here "index" is the row number clicked inside the ResultsView
                    if (model.trigger(index)) {
                        resultTriggered();
                    }
                }
            }

            onItemAdded: updateTabOrderRequested()
            onItemRemoved: updateTabOrderRequested()
        }
    }

    Component {
        id: resultsViewComponent
        ResultsView {
            width: parent.width
            typeAhead: main.typeAhead

            onIndexClicked: {
                main.typeAhead = "";
                if (model.trigger(index)) {
                    resultTriggered();
                }
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
        }
    }

    // Ui
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
            var tokens = source.split(":");
            var modelName = tokens[0];
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

            if (tokens.length == 2) {
                if ("arguments" in model) {
                    model.arguments = tokens[1].split(",");
                } else {
                    console.log("Error: trying to set arguments on model " + model + ", which does not support arguments");
                }
            }
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

    function createPage(models) {
        if (page) {
            page.destroy();
        }
        page = pageComponent.createObject(main);
        models.forEach(function(model) {
            var component = "modelForRow" in model ? multiResultsViewComponent : resultsViewComponent;
            component.createObject(page.viewContainer, {"model": model, "favoriteModels": favoriteModels});
        });

        updateTabOrderRequested();
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
