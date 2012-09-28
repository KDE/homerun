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
import org.kde.homerun.components 0.1 as HomerunComponents
import org.kde.homerun.fixes 0.1 as HomerunFixes
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1 as QtExtra

import "KeyboardUtils.js" as KeyboardUtils

import "TabContentInternal.js" as TabContentInternal

Item {
    id: main

    //- Public ----------------------------------------------------
    // Defined by outside world
    property QtObject sourceRegistry
    property variant sources
    property string searchCriteria

    // Exposed by ourself
    property bool canGoBack: false
    property bool canGoForward: false
    property Item currentPage

    signal startedApplication
    signal updateTabOrderRequested
    signal setSearchFieldRequested(string text)

    //- Private ---------------------------------------------------
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
        Repeater {
            id: repeater
            property variant favoriteModels
            property bool modelNeedsFiltering: false
            delegate: ResultsView {
                width: repeater.parent.width

                property QtObject sourceModel: repeater.model.modelForRow(index)

                model: repeater.modelNeedsFiltering ? createFilterForModel(sourceModel) : sourceModel
                favoriteModels: repeater.favoriteModels

                onIndexClicked: {
                    handleTriggerResult(model.trigger(index));
                }
            }
        }
    }

    Component {
        id: pageComponent
        Page {
            anchors.fill: parent
        }
    }

    // Ui
    Item {
        // navRow = back|previous + breadcrumbs
        id: navRow
        property int maxHeight: 32
        height: canGoBack ? maxHeight : 0
        Behavior on height {
            NumberAnimation {
                duration: 200
                easing.type: Easing.OutQuad
            }
        }
        clip: true

        anchors {
            left: parent.left
            top: parent.top
            right: parent.right
        }

        PlasmaComponents.ToolButton {
            id: backButton
            width: height
            height: navRow.maxHeight

            flat: false
            iconSource: "go-previous"
            onClicked: goBack()
        }

        PlasmaComponents.ToolButton {
            id: forwardButton
            anchors {
                left: backButton.right
            }
            width: height
            height: navRow.maxHeight
            enabled: canGoForward

            flat: false
            iconSource: "go-next"
            onClicked: goForward()
        }

        Row {
            id: breadcrumbRow
            anchors {
                left: forwardButton.right
                leftMargin: 12
            }
            height: navRow.maxHeight
            Repeater {
                id: breadcrumbRepeater
                model: currentPage.pathModel
                delegate: PlasmaComponents.ToolButton {
                    height: breadcrumbRow.height
                    property string source: model.source

                    flat: false
                    checked: model.index == currentPage.pathModel.count - 1
                    enabled: !checked
                    text: model.display
                    onClicked: openSource(model.source)
                }
            }
        }
    }

    Item {
        id: pageContainer
        anchors {
            left: parent.left
            top: navRow.bottom
            right: parent.right
            bottom: parent.bottom
        }

        PlasmaCore.SvgItem {
            id: hline
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
            height: naturalSize.height
            z: 1000
            svg: PlasmaCore.Svg {
                imagePath: "widgets/scrollwidget"
            }
            elementId: "border-top"
        }
    }

    // Scripting
    Component.onCompleted: {
        var page = createPage(sources);
        TabContentInternal.addPage(page);
        TabContentInternal.goToLastPage();
    }

    onActiveFocusChanged: {
        if (activeFocus) {
            var item = KeyboardUtils.findFirstTabMeChildren(main);
            if (item !== null) {
                item.forceActiveFocus();
            }
        }
    }

    onSearchCriteriaChanged: {
        currentPage.searchCriteria = searchCriteria;
    }

    onCurrentPageChanged: {
        setSearchFieldRequested(currentPage.searchCriteria);
    }

    function goBack() {
        TabContentInternal.goBack();
    }

    function goForward() {
        TabContentInternal.goForward();
    }

    function goUp() {
        var count = breadcrumbRepeater.count;
        var source;
        if (count >= 2) {
            // count - 1 is the breadcrumb for the current content
            // count - 2 is the breadcrumb for the content up
            source = breadcrumbRepeater.itemAt(count - 2).source;
        }
        if (source !== null) {
            openSource(source);
        }
    }

    function handleTriggerResult(result) {
        if (result) {
            startedApplication();
            return;
        }
    }

    function openSource(source) {
        var page = createPage([source], { "showHeader": false });
        page.pathModel = page.models[0].pathModel;
        TabContentInternal.addPage(page);
        TabContentInternal.goToLastPage();
    }

    Keys.onPressed: {
        KeyboardUtils.processShortcutList([
            [Qt.AltModifier, Qt.Key_Left, goBack],
            [Qt.AltModifier, Qt.Key_Right, goForward],
            [Qt.AltModifier, Qt.Key_Up, goUp],
            ], event);
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

    function createPage(sources, viewExtraArgs /*= {}*/) {
        var page = pageComponent.createObject(pageContainer);

        // Create models
        var models = [];
        sources.forEach(function(x) {
            var model = createModelForSource(x, page);
            if (model) {
                models.push(model);
            }
        });
        page.models = models;

        // Create views
        var firstView = null;
        models.forEach(function(model) {
            var isMultiViewModel = "modelForRow" in model;
            var modelNeedsFiltering = !("query" in model);

            var viewArgs = {};
            viewArgs["favoriteModels"] = sourceRegistry.favoriteModels;
            viewArgs["model"] = model;
            if (modelNeedsFiltering) {
                if (isMultiViewModel) {
                    viewArgs["modelNeedsFiltering"] = true;
                } else {
                    viewArgs["model"] = createFilterForModel(model);
                }
            }
            if (viewExtraArgs !== undefined) {
                for (var key in viewExtraArgs) {
                    viewArgs[key] = viewExtraArgs[key];
                }
            }

            var component = isMultiViewModel ? multiResultsViewComponent : resultsViewComponent;
            var obj = component.createObject(page.viewContainer, viewArgs);
            if (!isMultiViewModel && firstView === null) {
                // Check isMultiViewModel because in that case obj is not a ResultsView
                firstView = obj;
            }

            // Useful for debugging
            page.objectName += model.objectName + ",";
        });

        page.finishModelConnections();

        if (firstView) {
            firstView.forceActiveFocus();
        }

        return page;
    }

    function createFilterForModel(model) {
        return genericFilterComponent.createObject(model, {"sourceModel": model});
    }

    function reset() {
        TabContentInternal.goTo(0);
        TabContentInternal.clearHistoryAfterCurrentPage();
    }
}
