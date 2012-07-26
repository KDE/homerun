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

    //- Public ----------------------------------------------------
    // Defined by outside world
    property variant favoriteModels
    property variant sources
    property string typeAhead
    property string searchCriteria

    // Exposed by ourself
    property bool searchable: searchModels.length > 0
    property bool canGoBack: false
    property bool canGoForward: false

    signal startedApplication
    signal updateTabOrderRequested

    function goBack() {
        TabContentInternal.goBack();
    }

    function goForward() {
        TabContentInternal.goForward();
    }

    //- Private ---------------------------------------------------
    property variant browseModels: []
    property variant searchModels: []

    property bool componentCompleted: false

    property Item currentPage

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
            property Item firstView

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

    Item {
        id: headerRow
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
            height: headerRow.maxHeight

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
            height: headerRow.maxHeight
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
            height: headerRow.maxHeight
            Repeater {
                id: repeater
                model: (currentPage.firstView && currentPage.firstView.model.pathModel) ? currentPage.firstView.model.pathModel : null
                delegate: PlasmaComponents.ToolButton {
                    height: breadcrumbRow.height

                    flat: false
                    checked: model.index == repeater.model.count - 1
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
            top: headerRow.bottom
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
        createPage(models, { "showHeader": false });
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

    function createPage(models, viewExtraArgs /*= {}*/) {
        var page = pageComponent.createObject(pageContainer);
        models.forEach(function(model) {
            var component = "modelForRow" in model ? multiResultsViewComponent : resultsViewComponent;
            var viewArgs = {};
            viewArgs["model"] = model;
            viewArgs["favoriteModels"] = favoriteModels;
            if (viewExtraArgs !== undefined) {
                for (var key in viewExtraArgs) {
                    viewArgs[key] = viewExtraArgs[key];
                }
            }
            var view = component.createObject(page.viewContainer, viewArgs);
            if (page.firstView === null) {
                page.firstView = view;
            }
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
