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

import Qt 4.7
import org.kde.sal.components 0.1 as SalComponents
import org.kde.sal.fixes 0.1 as SalFixes
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents

import "KeyboardUtils.js" as KeyboardUtils

FocusScope {
    id: main

    property QtObject favoriteModel
    property variant sources

    signal resultTriggered

    // Internal
    property variant models: []

    // Models
    Component {
        id: serviceModelComponent
        SalComponents.SalServiceModel {
            path: "/"
            property string name: "Applications"
            function favoriteAction(obj) {
                return obj.entryPath === undefined ? "" : "add";
            }

            function triggerFavoriteAction(obj) {
                if (obj.entryPath === undefined) {
                    return;
                }
                var serviceId = obj.entryPath.replace(/.*\//, ""); // Keep only filename
                favoriteModel.append(serviceId);
            }
        }
    }

    Component {
        id: powerModelComponent
        SalFixes.SortFilterModel {
            property string name: "Power"
            filterRegExp: searchField.text

            sourceModel: SalComponents.PowerModel {
                id: realPowerModel
            }

            function favoriteAction(obj) {
                return "";
            }

            function triggerFavoriteAction(obj) {
                var sourceRow = mapRowToSource(obj.index);
                sourceModel.removeAt(sourceRow);
            }

            function trigger(row) {
                return sourceModel.trigger(mapRowToSource(row));
            }
        }
    }


    Component {
        id: runnerModelComponent
        RunnerModel {
            query: searchField.text
            favoriteModel: main.favoriteModel
        }
    }

    Component {
        id: favoriteModelComponent
        SalFixes.SortFilterModel {
            property string name: "Favorite Applications"
            filterRegExp: searchField.text

            sourceModel: main.favoriteModel

            function favoriteAction(obj) {
                return "remove";
            }

            function triggerFavoriteAction(obj) {
                var sourceRow = mapRowToSource(obj.index);
                sourceModel.removeAt(sourceRow);
            }

            function trigger(row) {
                return sourceModel.trigger(mapRowToSource(row));
            }
        }
    }

    Component {
        id: placesModelComponent
        SalComponents.PlacesModel {
            id: placesFilterModel
            property string name: "Favorite Places"
            filter: searchField.text

            function favoriteAction(obj) {
                return obj.favoriteAction;
            }

            function triggerFavoriteAction(obj) {
                triggerFavoriteActionAt(obj.index);
            }
        }
    }

    // UI
    Component {
        id: resultsViewComponent
        ResultsView {
            width: parent.width
            visible: model.count > 0

            onIndexClicked: {
                if (model.trigger(index)) {
                    resultTriggered();
                }
            }
        }
    }

    PlasmaComponents.TextField {
        id: searchField

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        focus: true
        clearButtonShown: true
    }

    Flickable {
        id: resultsFlickable
        anchors {
            top: searchField.bottom
            topMargin: 12
            bottom: parent.bottom
            left: parent.left
            right: scrollBar.left
        }
        contentWidth: width
        contentHeight: resultsColumn.height
        clip: true
        SalComponents.SalRunnerModel {
            id: salRunnerModel
        }
        Column {
            id: resultsColumn
            width: parent.width
            Repeater {
                id: repeater
                model: salRunnerModel
                delegate:
                    ResultsView {
                        width: parent.width
                        model: salRunnerModel.modelForRow(index) // Here "index" is the current row number within the repeater
                        favoriteModel: main.favoriteModel
                        onIndexClicked: {
                            // Here "index" is the row number clicked inside the ResultsView
                            if (model.trigger(index)) {
                                resultTriggered();
                            }
                        }
                    }
            }
        }
    }

    PlasmaComponents.ScrollBar {
        id: scrollBar
        flickableItem: resultsFlickable
        anchors {
            right: parent.right
            top: searchField.bottom
            bottom: parent.bottom
        }
    }

    // Scripting
    Component.onCompleted: {
        var views = createResultsViews();
        var widgets = [searchField].concat(views).concat(searchField);
        KeyboardUtils.setTabOrder(widgets);
    }

    function createResultsViews() {
        var idx;
        var lst = new Array();
        var views = new Array();
        for (idx = 0; idx < sources.length; ++idx) {
            // Create models
            var tokens = sources[idx].split(":");
            var modelName = tokens[0];
            var model;
            if (modelName == "ServiceModel") {
                model = serviceModelComponent.createObject(main);
            } else if (modelName == "PlacesModel") {
                model = placesModelComponent.createObject(main);
            } else if (modelName == "FavoriteModel") {
                model = favoriteModelComponent.createObject(main);
            } else if (modelName == "PowerModel") {
                model = powerModelComponent.createObject(main);
            } else {
                model = runnerModelComponent.createObject(main);
            }

            if (tokens.length == 2) {
                if ("arguments" in model) {
                    model.arguments = tokens[1].split(",");
                } else {
                    console.log("Error: trying to set arguments on model " + model + ", which does not support arguments");
                }
            }

            // Create views
            var newViews = new Array();
            var view = resultsViewComponent.createObject(resultsColumn, {"model": model, "favoriteModel": favoriteModel});
            newViews.push(view);

            // Define KeyNavigation for views
            newViews.forEach(function(view) {
                var previousItem = views.length > 0 ? views[views.length - 1] : searchField;
                previousItem.KeyNavigation.down = view;
                view.KeyNavigation.up = previousItem;
                views.push(view);
            });

            lst.push(model);
        }
        models = lst;

        return views;
    }

    function reset() {
        searchField.text = "";
        models.forEach(function(model) {
            if ("path" in model) {
                model.path = "/";
            }
        });
    }
}
