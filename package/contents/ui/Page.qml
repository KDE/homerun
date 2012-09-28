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
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.homerun.fixes 0.1 as HomerunFixes

Item {
    id: main

    //- Defined by outside world -----------------------------------
    property QtObject sourceRegistry
    property variant sources

    property string searchCriteria

    //- Read-write properties --------------------------------------
    property bool showHeader: true

    //- Read-only properties ---------------------------------------
    // Defined for pages with a single view on a browsable model
    property QtObject pathModel: models.length == 1 ? models[0].pathModel : null

    //- Private properties -----------------------------------------
    property list<QtObject> models

    //- Components -------------------------------------------------
    Component {
        id: runningConnectionComponent
        Connections {
            ignoreUnknownSignals: true
            onRunningChanged: main.updateRunning()
        }
    }

    // Filter components
    Component {
        id: genericFilterComponent
        HomerunFixes.SortFilterModel {
            filterRegExp: main.searchCriteria
            property string name: sourceModel.name
            property int count: sourceModel.count

            property bool running: "running" in sourceModel ? sourceModel.running : false
            property QtObject pathModel: "pathModel" in sourceModel ? sourceModel.pathModel : null

            function trigger(index) {
                var sourceIndex = mapRowToSource(index);
                sourceModel.trigger(sourceIndex);
            }

        }
    }

    Component {
        id: openSourceConnectedConnectionComponent
        Connections {
            ignoreUnknownSignals: true
            onOpenSourceRequested: {
                openSource(source);
            }
        }
    }

    Component {
        id: queryBindingComponent
        Binding {
            property: "query"
            value: main.searchCriteria
        }
    }

    // UI components
    Component {
        id: resultsViewComponent
        ResultsView {
            id: view
            width: parent.width
            onIndexClicked: {
                handleTriggerResult(model.trigger(index));
            }
        }
    }

    Component {
        id: multiResultsViewComponent
        Column {
            id: multiMain
            width: parent.width
            property alias model: repeater.model
            property bool showHeader

            property bool modelNeedsFiltering: false
            property variant favoriteModels

            Repeater {
                id: repeater
                delegate: ResultsView {
                    width: multiMain.width

                    model: multiMain.modelNeedsFiltering
                        ? createFilterForModel(repeater.model.modelForRow(index))
                        : sourceModel
                    favoriteModels: multiMain.favoriteModels

                    onIndexClicked: {
                        handleTriggerResult(model.trigger(index));
                    }
                }
            }
        }
    }

    //- UI ---------------------------------------------------------
    PlasmaComponents.BusyIndicator {
        id: busyIndicator
        anchors {
            horizontalCenter: parent.horizontalCenter
        }
        y: 12

        opacity: running ? 0.5 : 0
    }

    Flickable {
        id: flickable
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
            right: scrollBar.left
        }
        contentWidth: width
        contentHeight: viewContainer.height
        clip: true
        Column {
            id: viewContainer
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

    //- Code -------------------------------------------------------
    function getFirstView() {
        var lst = KeyboardUtils.findTabMeChildren(this);
        return lst.length > 0 ? lst[0] : null;
    }

    function updateRunning() {
        for (var idx = 0; idx < models.length; ++idx) {
            if (models[idx].running) {
                busyIndicator.running = true;
                return;
            }
        }
        busyIndicator.running = false;
    }

    function finishModelConnections() {
        for (var idx = 0; idx < models.length; ++idx) {
            var model = models[idx];
            runningConnectionComponent.createObject(main, {"target": model});
        }
        main.updateRunning();
    }

    function createFilterForModel(model) {
        if (!model) {
            console.log("createFilterForModel: invalid model");
            return null;
        }
        return genericFilterComponent.createObject(model, {"sourceModel": model});
    }

    function createModelForSource(source, parent) {
        var model = sourceRegistry.createModelForSource(source, parent);
        if (!model) {
            return null;
        }

        // Create connections now: if we do it after applying the filter, then
        // "model" may have been changed to be a filter model, not the source
        // model
        openSourceConnectedConnectionComponent.createObject(model);

        if ("query" in model) {
            // Model supports querying itself, bind the search criteria field to its "query" property
            queryBindingComponent.createObject(main, {"target": model});
        }

        return model;
    }

    Component.onCompleted: {
        // Create models
        var models = [];
        sources.forEach(function(x) {
            var model = createModelForSource(x, main);
            if (model) {
                models.push(model);
            }
        });
        main.models = models;

        // Create views
        var firstView = null;
        models.forEach(function(model) {
            var isMultiViewModel = "modelForRow" in model;
            var modelNeedsFiltering = !("query" in model);

            var viewArgs = {};
            viewArgs["favoriteModels"] = sourceRegistry.favoriteModels;
            viewArgs["model"] = model;
            viewArgs["showHeader"] = showHeader;
            if (modelNeedsFiltering) {
                if (isMultiViewModel) {
                    viewArgs["modelNeedsFiltering"] = true;
                } else {
                    viewArgs["model"] = createFilterForModel(model);
                }
            }

            var component = isMultiViewModel ? multiResultsViewComponent : resultsViewComponent;
            var obj = component.createObject(viewContainer, viewArgs);
            if (!isMultiViewModel && firstView === null) {
                // Check isMultiViewModel because in that case obj is not a ResultsView
                firstView = obj;
            }

            // Useful for debugging
            main.objectName += model.objectName + ",";
        });

        finishModelConnections();

        if (firstView) {
            firstView.forceActiveFocus();
        }
    }
}
