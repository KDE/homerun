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
    property variant favoriteModels
    property variant sources
    property variant searchSources
    property string typeAhead
    property string searchCriteria

    // Exposed by ourself
    property bool canGoBack: false
    property bool canGoForward: false
    property Item currentPage

    signal startedApplication
    signal updateTabOrderRequested
    signal setSearchFieldRequested(string text)

    //- Private ---------------------------------------------------
    property Item searchPage

    // Used to restore content of search field when bringing back the searchPage from history
    property string lastSearchCriteria

    HomerunComponents.SharedConfig {
        id: config
        name: "homerunrc"
    }

    // Models
    Component {
        id: serviceModelComponent
        HomerunComponents.ServiceModel {
            installer: config.readEntry("PackageManagement", "categoryInstaller")

            onOpenSourceRequested: {
                openSource(source);
            }
        }
    }

    Component {
        id: powerModelComponent
        HomerunComponents.PowerModel {
        }
    }

    Component {
        id: sessionModelComponent
        HomerunComponents.SessionModel {
        }
    }

    Component {
        id: runnerModelComponent
        HomerunComponents.RunnerModel {
            query: searchCriteria
        }
    }

    Component {
        id: placesModelComponent
        HomerunComponents.PlacesModel {
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
                width: repeater.parent.width
                model: repeater.model.modelForRow(index) // Here "index" is the current row number within the repeater
                typeAhead: main.typeAhead
                favoriteModels: repeater.favoriteModels
                onIndexClicked: {
                    // Here "index" is the row number clicked inside the ResultsView
                    handleTriggerResult(model.trigger(index));
                }
            }
        }
    }

    Component {
        id: resultsViewComponent
        ResultsView {
            id: view
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
            id: pageMain
            property alias viewContainer: column
            anchors.fill: parent

            // Defined for pages with a single view on a browsable model
            property QtObject pathModel

            property list<QtObject> models

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

            Component {
                id: runningConnectionComponent
                Connections {
                    ignoreUnknownSignals: true
                    onRunningChanged: pageMain.updateRunning()
                }
            }

            Component.onCompleted: {
                for (var idx = 0; idx < models.length; ++idx) {
                    var model = models[idx];
                    runningConnectionComponent.createObject(pageMain, {"target": model});
                }
                pageMain.updateRunning();
            }
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
                    text: model.label
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
            bottom: filterRow.top
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

    Item {
        id: filterRow
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        property int maxHeight: typeAheadLabel.height + filterLine.height
        height: typeAhead == "" ? 0 : maxHeight
        Behavior on height {
            NumberAnimation {
                duration: 200
                easing.type: Easing.OutQuad
            }
        }
        clip: true

        PlasmaCore.SvgItem {
            id: filterLine
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
            height: naturalSize.height
            svg: PlasmaCore.Svg {
                imagePath: "widgets/scrollwidget"
            }
            elementId: "border-bottom"
        }

        PlasmaComponents.Label {
            id: typeAheadLabel
            anchors {
                horizontalCenter: parent.horizontalCenter
                top: filterLine.bottom
            }
            text: typeAhead + "|"
            font.pointSize: theme.defaultFont.pointSize * 1.4
        }
    }

    // Scripting
    Component.onCompleted: {
        var lst = sources.map(createModelForSource);
        var page = createPage(lst);
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
        if (searchCriteria.length > 0) {
            // User typed a new search
            if (searchPage) {
                if (currentPage !== searchPage) {
                    TabContentInternal.goToPage(searchPage);
                }
            } else {
                var lst = searchSources.map(createModelForSource);
                searchPage = createPage(lst);
                TabContentInternal.addPage(searchPage);
                TabContentInternal.goToLastPage();
            }
            lastSearchCriteria = searchCriteria;
        } else {
            if (currentPage === searchPage) {
                // User cleared search field himself
                goBack();
            }
        }
    }

    onCurrentPageChanged: {
        if (currentPage !== searchPage && searchCriteria.length > 0) {
            setSearchFieldRequested("");
        }
    }

    function goBack() {
        TabContentInternal.goBack();
    }

    function goForward() {
        TabContentInternal.goForward();
        if (currentPage === searchPage) {
            // Restore search query
            setSearchFieldRequested(lastSearchCriteria);
        }
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
        main.typeAhead = "";
        if (result) {
            startedApplication();
            return;
        }
    }

    function openSource(source) {
        var models = [createModelForSource(source)];
        var page = createPage(models, { "showHeader": false });
        page.pathModel = models[0].pathModel;
        TabContentInternal.addPage(page);
        TabContentInternal.goToLastPage();
    }

    Keys.onPressed: {
        if (event.modifiers == Qt.NoModifier || event.modifiers == Qt.ShiftModifier) {
            handleTypeAheadKeyEvent(event);
        }
        KeyboardUtils.processShortcutList([
            [Qt.AltModifier, Qt.Key_Left, goBack],
            [Qt.AltModifier, Qt.Key_Right, goForward],
            [Qt.AltModifier, Qt.Key_Up, goUp],
            ], event);
    }

    function handleTypeAheadKeyEvent(event) {
        switch (event.key) {

        case Qt.Key_Tab:
        case Qt.Key_Escape:
            // Keys we don't want to handle as type-ahead
            return;
        case Qt.Key_Backspace:
            // Erase last char
            typeAhead = typeAhead.slice(0, -1);
            event.accepted = true;
            break;
        default:
            // Add the char to typeAhead
            if (event.text != "") {
                typeAhead += event.text;
                event.accepted = true;
            }
            break;
        }
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
        model.objectName = source;

        if (modelArgs) {
            if ("arguments" in model) {
                model.arguments = modelArgs;
            } else {
                console.log("Error: trying to set arguments on model " + model + ", which does not support arguments");
            }
        }
        return model;
    }

    function createPage(models, viewExtraArgs /*= {}*/) {
        var page = pageComponent.createObject(pageContainer, {"models": models});
        var firstView = null;
        models.forEach(function(model) {
            var isMultiViewModel = "modelForRow" in model;
            var component = isMultiViewModel ? multiResultsViewComponent : resultsViewComponent;
            var viewArgs = {};
            viewArgs["model"] = model;
            viewArgs["favoriteModels"] = favoriteModels;
            if (viewExtraArgs !== undefined) {
                for (var key in viewExtraArgs) {
                    viewArgs[key] = viewExtraArgs[key];
                }
            }
            var obj = component.createObject(page.viewContainer, viewArgs);
            if (!isMultiViewModel && firstView === null) {
                // Check isMultiViewModel because in that case obj is not a ResultsView
                firstView = obj;
            }
            page.objectName += model.objectName + ",";
        });

        if (firstView) {
            firstView.forceActiveFocus();
        }
        return page;
    }

    function reset() {
        TabContentInternal.goTo(0);
    }
}
