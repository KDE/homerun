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
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.homerun.fixes 0.1 as HomerunFixes

Item {
    id: main

    //- Defined by outside world -----------------------------------
    property QtObject sourceRegistry
    property variant sources
    property bool configureMode

    property string searchCriteria

    //- Read-write properties --------------------------------------
    property bool showHeader: true

    //- Read-only properties ---------------------------------------
    // Defined for pages with a single view on a browsable model
    property QtObject pathModel: sourcesModel.count == 1 ? sourcesModel.get(0).model.pathModel : null

    signal sourcesUpdated(variant sources)

    //- Non visual elements ----------------------------------------
    ListModel {
        id: sourcesModel
    }

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
                return sourceModel.trigger(sourceIndex);
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
                        : repeater.model.modelForRow(index)
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

    ListView {
        id: availableSourcesView
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
        width: configureMode ? parent.width * 0.2 : 0
        opacity: configureMode ? 1 : 0
        Behavior on width { NumberAnimation {} }
        Behavior on opacity { NumberAnimation {} }

        model: HomerunFixes.SortFilterModel {
            sourceModel: sourceRegistry.availableSourcesModel()
            sortRole: "display"
        }

        delegate: PlasmaComponents.Button {
            width: parent.width - 24
            text: model.display
            onClicked: {
                addSource(model.sourceId);
                main.updateSources();
            }
        }

        PlasmaCore.SvgItem {
            anchors {
                right: parent.right
                rightMargin: 12
                top: parent.top
                bottom: parent.bottom
            }
            width: naturalSize.width
            z: 1000
            svg: PlasmaCore.Svg {
                imagePath: "widgets/scrollwidget"
            }
            elementId: "border-right"
        }
    }

    ListView {
        id: listView
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: availableSourcesView.right
            right: scrollBar.left
        }
        clip: true
        model: sourcesModel
        delegate: SourceItem {
            id: editorMain
            width: parent ? parent.width : 0
            configureMode: main.configureMode
            sourceRegistry: main.sourceRegistry
            sourceId: model.sourceId
            property QtObject view

            isFirst: model.index == 0
            isLast: model.index == sourcesModel.count - 1

            onRemoveRequested: {
                sourcesModel.remove(model.index);
                main.updateSources();
            }
            onMoveRequested: {
                sourcesModel.move(model.index, model.index + delta, 1);
                main.updateSources();
            }

            onSourceIdChanged: {
                model.model.destroy()
                view.destroy()
                var newModel = createModelForSource(sourceId, main);
                sourcesModel.setProperty(model.index, "sourceId", sourceId);
                sourcesModel.setProperty(model.index, "model", newModel);
                view = createView(model.model, editorMain);
                main.updateSources();
            }

            Component.onCompleted: {
                view = createView(model.model, editorMain);
                main.updateRunning();
            }
        }
    }

    PlasmaComponents.ScrollBar {
        id: scrollBar
        flickableItem: listView
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

    Component.onCompleted: fillSourcesModel()

    //- Code -------------------------------------------------------
    function getFirstView() {
        var lst = KeyboardUtils.findTabMeChildren(this);
        return lst.length > 0 ? lst[0] : null;
    }

    function updateRunning() {
        for (var idx = 0; idx < sourcesModel.count; ++idx) {
            if (sourcesModel.get(idx).model.running) {
                busyIndicator.running = true;
                return;
            }
        }
        busyIndicator.running = false;
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

        openSourceConnectedConnectionComponent.createObject(model);

        if ("query" in model) {
            // Model supports querying itself, bind the search criteria field to its "query" property
            queryBindingComponent.createObject(main, {"target": model});
        }

        runningConnectionComponent.createObject(main, {"target": model});
        return model;
    }

    function createView(model, parent) {
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
        return component.createObject(parent, viewArgs);
    }

    function fillSourcesModel() {
        sources.forEach(addSource);
    }

    function updateSources() {
        var lst = new Array();
        for (var idx = 0; idx < sourcesModel.count; ++idx) {
            var item = sourcesModel.get(idx);
            lst.push(item.sourceId);
        }
        sourcesUpdated(lst);
    }

    function addSource(sourceId) {
        var model = createModelForSource(sourceId, main);
        if (!model) {
            console.log("addSource() could not create model for source: " + sourceId);
            return;
        }
        sourcesModel.append({
            sourceId: sourceId,
            model: model,
        });
    }
}
