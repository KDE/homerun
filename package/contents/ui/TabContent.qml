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
import org.kde.plasma.components 0.1 as PlasmaComponents

FocusScope {
    id: root
    property QtObject favoriteModel
    property variant sources

    Component {
        id: serviceModelComponent
        SalComponents.SalServiceModel {
            path: "/"
            property string name: "Applications"

            function serviceIdForObject(obj) {
                if (obj.entryPath === undefined) {
                    return undefined;
                }
                return obj.entryPath.replace(/.*\//, ""); // Keep only filename
            }
        }
    }

    Component {
        id: runnerModelComponent
        RunnerModel {
            query: searchField.text
        }
    }

    Component {
        id: placesModelComponent
        SalComponents.PlacesModel {
            property string name: "Places"
        }
    }

    Component {
        id: resultsViewComponent
        ResultsView {
            width: parent.width
            visible: model.count > 0

            onIndexClicked: {
                model.run(index);
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
        Column {
            id: resultsColumn
            width: parent.width
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

    Component.onCompleted: {
        var idx;
        for (idx = 0; idx < sources.length; ++idx) {
            var tokens = sources[idx].split(":");
            var modelName = tokens[0];
            var model;
            if (modelName == "ServiceModel") {
                model = serviceModelComponent.createObject(root);
            } else if (modelName == "PlacesModel") {
                model = placesModelComponent.createObject(root);
            } else if (modelName == "FavoriteModel") {
                model = root.favoriteModel;
            } else {
                model = runnerModelComponent.createObject(root);
            }
            if (tokens.length == 2) {
                var args = tokens[1].split(",");
                model.runners = args;
                model.name = args[0];
            }
            resultsViewComponent.createObject(resultsColumn, {"model": model, "favoriteModel": favoriteModel});
        }
    }
}
