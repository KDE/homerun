/*
Copyright 2012 Aurélien Gâteau <agateau@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
import QtQuick 1.1
import org.kde.homerun.components 0.1 as HomerunComponents
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.extras 0.1 as PlasmaExtras
import org.kde.qtextracomponents 0.1 as QtExtra
import org.kde.homerun.fixes 0.1 as HomerunFixes

Item {
    id: main

    //- Defined by outside world -----------------------------------
    property Item rootItem
    property QtObject sourceRegistry

    property QtObject tabSourceModel
    property bool configureMode
    property bool showActionListOverlay

    property string searchCriteria

    //- Read-write properties --------------------------------------
    property bool showHeader: true

    //- Read-only properties ---------------------------------------
    // Defined for pages with a single view on a browsable model
    property QtObject pathModel: tabSourceModel.count == 1 ? tabSourceModel.get(0).model.pathModel : null

    signal closeRequested()
    signal openSourceRequested(string sourceId, variant sourceArguments)
    signal showMessageRequested(string icon, string text)

    property Item previouslyFocusedItem

    //- Components -------------------------------------------------
    // Filter components
    Component {
        id: genericFilterComponent

        HomerunFixes.SortFilterModel {
            filterRegExp: main.searchCriteria
            property string name: sourceModel.name
            property int count: sourceModel.count
            property bool canMoveRow: "canMoveRow" in sourceModel ? sourceModel.canMoveRow : false
            property bool running: "running" in sourceModel ? sourceModel.running : false
            property QtObject pathModel: "pathModel" in sourceModel ? sourceModel.pathModel : null

            objectName: "SortFilterModel:" + (sourceModel ? sourceModel.objectName : "")

            function trigger(index, actionId, actionArgument) {
                var sourceIndex = mapRowToSource(index);
                return sourceModel.trigger(sourceIndex, actionId, actionArgument);
            }

            function moveRow(from, to) {
                if (!canMoveRow) {
                    console.log("Page.qml: moveRow(): source model cannot move rows. This method should not be called.");
                    return;
                }
                var sourceFrom = mapRowToSource(from);
                var sourceTo = mapRowToSource(to);
                sourceModel.moveRow(sourceFrom, sourceTo);
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
            configureMode: main.configureMode
            showActionListOverlay: main.showActionListOverlay
            onTriggerActionRequested: {
                handleTriggerResult(model.trigger(index, actionId, actionArgument));
            }
            onCurrentItemChanged: {
                if (!view.visible) {
                    return;
                }

                if (currentItem) {
                    main.scrollToItem(currentItem);
                }
            }
            onShowMessageRequested: {
                main.showMessageRequested(icon, text);
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

            // Expose the same focus API as ResultsView. Used when focus changes
            // from a single ResultsView source to a multi ResultsView source
            // and vice-versa
            signal focusOtherViewRequested(int key, int x)

            function focusLastItem() {
                lastView().focusLastItem();
            }

            function focusFirstItem() {
                firstView().focusFirstItem();
            }

            function focusLastItemAtX(x) {
                lastView().focusLastItemAtX(x);
            }

            function focusFirstItemAtX(x) {
                firstView().focusFirstItemAtX(x);
            }

            function isEmpty() {
                for (var idx = 0; idx < repeater.count; ++idx) {
                    if (!repeater.itemAt(idx).isEmpty()) {
                        return false;
                    }
                }
                return true;
            }

            function focusedItem() {
                for (var idx = 0; idx < repeater.count; ++idx) {
                    var view = repeater.itemAt(idx);
                    var item = view.focusedItem();
                    if (item) {
                        return item;
                    }
                }
                return null;
            }

            function triggerFirstItem() {
                for (var idx = 0; idx < repeater.count; ++idx) {
                    var view = repeater.itemAt(idx);
                    if (!view.isEmpty()) {
                        view.triggerFirstItem();
                        return;
                    }
                }
            }

            Repeater {
                id: repeater
                delegate: ResultsView {
                    width: multiMain.width
                    configureMode: main.configureMode
                    showActionListOverlay: main.showActionListOverlay

                    model: multiMain.modelNeedsFiltering
                        ? createFilterForModel(repeater.model.modelForRow(index))
                        : repeater.model.modelForRow(index)
                    favoriteModels: multiMain.favoriteModels

                    onModelChanged: {
                        if ("applicationLaunched" in model) {
                            model.applicationLaunched.connect(rootItem.applicationLaunched);
                        }
                    }

                    onTriggerActionRequested: {
                        handleTriggerResult(model.trigger(index, actionId, actionArgument));
                    }

                    onFocusOtherViewRequested: {
                        if (!main.navigate(repeater, index, key, x)) {
                            // No view matches, forward the request
                            multiMain.focusOtherViewRequested(key, x);
                        }
                    }

                    onCurrentItemChanged: {
                        if (!visible) {
                            return;
                        }

                        if (currentItem) {
                            main.scrollToItem(currentItem);
                        }
                    }
                    onShowMessageRequested: {
                        main.showMessageRequested(icon, text);
                    }
                }

                function viewAt(idx) {
                    return itemAt(idx);
                }
            }

            function firstView() {
                return repeater.itemAt(0);
            }

            function lastView() {
                return repeater.itemAt(repeater.count - 1);
            }
        }
    }

    Component {
        id: sideBarListItem

        PlasmaCore.FrameSvgItem {
            width: parent.width
            height: theme.defaultFont.mSize.height * 2.8

            imagePath: "widgets/listitem"
            prefix: model.checked ? "pressed" : ""

            HomerunComponents.Label {
                height: parent.height

                anchors {
                    left: parent.left
                    leftMargin: 10
                    right: parent.right
                    rightMargin: 10
                }

                onImplicitWidthChanged: {
                    var fullWidth = Math.ceil(implicitWidth) + anchors.leftMargin + (anchors.rightMargin * 2);

                    if (fullWidth > sideBarScrollArea.textWidth) {
                        sideBarScrollArea.textWidth = fullWidth;
                    }
                }

                opacity: model.checked ? 1 : (isContainment? 0.8 : 0.4)

                text: model.display
                font.pointSize: theme.defaultFont.pointSize * 1.4
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true

                    onContainsMouseChanged: {
                        if (containsMouse) {
                            listView.currentIndex = index;
                        }

                        listView.highlightItem.opacity = containsMouse ? 1 : 0;
                    }

                    onClicked: {
                        listView.model.trigger(index);
                    }
                }
            }
        }
    }

    Component {
        id: sideBarList

        ListView {
            id: listView

            height: (theme.defaultFont.mSize.height * 2.8) * count
            contentHeight: height

            anchors {
                left: parent.left
                right: parent.right
                rightMargin: 10
            }

            delegate: sideBarListItem

            highlight: PlasmaComponents.Highlight {
                hover: true
            }

            highlightMoveDuration: 200
        }
    }

    //- UI ---------------------------------------------------------
    PlasmaExtras.ScrollArea {
        id: availableScrollArea
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
        width: configureMode ? parent.width * 0.2 : 0
        opacity: configureMode ? 1 : 0
        Behavior on width { NumberAnimation {} }
        Behavior on opacity { NumberAnimation {} }

        ListView {
            spacing: 6

            model: PlasmaCore.SortFilterModel {
                sourceModel: sourceRegistry.availableSourcesModel()
                sortRole: "display"
            }

            delegate: AvailableSourceItem {
                width: ListView.view.width
                text: model.display
                comment: model.comment
                onClicked: {
                    tabSourceModel.appendSource(sourceId);
                }
            }
        }
    }

    PlasmaComponents.BusyIndicator {
        id: busyIndicator
        y: 12
        anchors {
            horizontalCenter: parent.horizontalCenter
        }

        opacity: running ? 0.5 : 0
    }

    PlasmaExtras.ScrollArea {
        id: sideBarScrollArea

        property int sideBarCount: 0
        property int textWidth: 0

        width: visible ? textWidth : 0
        opacity: visible ? 1 : 0

        visible: sideBarCount > 0

        anchors {
            left: availableScrollArea.right
            top: parent.top
            bottom: parent.bottom
        }

        Behavior on width { NumberAnimation {} }
        Behavior on opacity { NumberAnimation {} }

        Flickable {
            id: sideBarFlickable
            anchors.fill: parent
            clip: true
            contentHeight: sideBarColumn.height

            Column {
                id: sideBarColumn
                width: parent.width

                Repeater {
                    id: sideBarRepeater
                    model: tabSourceModel

                    onItemRemoved: {
                        if ("sideBarModel" in item.viewModel) {
                            sideBarScrollArea.sideBarCount -= 1;
                        }
                    }

                    delegate: Item {
                        id: sideBarDelegateMain

                        width: parent.width
                        height: view.height

                        property QtObject view: Item {}
                        property QtObject viewModel: model.model

                        onViewModelChanged: {
                            if (view) {
                                view.destroy();
                            }

                            if ("sideBarModel" in model.model) {
                                view = sideBarList.createObject(sideBarDelegateMain,
                                    {"model": model.model.sideBarModel})
                                sideBarScrollArea.sideBarCount += 1;
                            }
                        }
                    }
                }
            }
        }
    }

    PlasmaExtras.ScrollArea {
        id: centralScrollArea
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: sideBarScrollArea.right
            right: parent.right
        }

        Flickable {
            id: centralFlickable
            contentHeight: centralColumn.height

            Column {
                id: centralColumn
                width: parent.width

                Repeater {
                    id: repeater
                    model: tabSourceModel
                    delegate: Column {
                        id: delegateMain
                        width: parent ? parent.width : 0
                        height: main.configureMode
                            ? Math.min(implicitHeight, 200)
                            : implicitHeight
                        clip: main.configureMode

                        property QtObject view: ResultsView {}
                        property string sourceId: model.sourceId
                        property int viewIndex: model.index
                        property QtObject viewModel: model.model

                        SourceItem {
                            width: parent.width
                            configureMode: main.configureMode
                            sourceRegistry: main.sourceRegistry
                            sourceId: delegateMain.sourceId

                            isFirst: viewIndex == 0
                            isLast: viewIndex == repeater.count - 1

                            onRemoveRequested: {
                                tabSourceModel.remove(delegateMain.viewIndex);
                            }
                            onMoveRequested: {
                                tabSourceModel.move(delegateMain.viewIndex, delegateMain.viewIndex + delta);
                            }

                            onConfigureRequested: {
                                var row = delegateMain.viewIndex;
                                var dlg = sourceRegistry.createConfigurationDialog(sourceId, model.configGroup);
                                if (!dlg.exec()) {
                                    return;
                                }
                                dlg.save();
                                tabSourceModel.recreateModel(row);
                            }
                        }

                        onViewModelChanged: {
                            createViewForRow();
                        }

                        function createViewForRow() {
                            connectModel(model.model);
                            if (view) {
                                view.destroy();
                            }
                            view = main.createView(model.model, delegateMain);
                            main.updateRunning();
                        }

                        function navigate(key, x) {
                            main.navigate(repeater, model.index, key, x);
                        }
                    }

                    function viewAt(idx) {
                        return itemAt(idx).view;
                    }
                }
            }
        }
    }


    Behavior on opacity {
        NumberAnimation {
            duration: 200
        }
    }

    Timer {
        id: focusTimer
        interval: 500
        repeat: true
        running: false
        property int count: 0

        onTriggered: {
            var maxCount = 10;
            console.log("Page.qml: focusTimer");
            updateFocus();
            if (main.focusedItem()) {
                console.log("Page.qml: focusTimer: Focus has been set");
                stop();
                return;
            }
            console.log("Page.qml: focusTimer: Still no valid focus");
            count += 1;
            if (count >= maxCount) {
                console.log("Page.qml: focusTimer: Giving up after " + maxCount + " tries");
                stop();
            }
        }
    }

    onActiveFocusChanged: {
        if (!activeFocus) {
            return;
        }
        updateFocus();
        if (!focusedItem()) {
            focusTimer.count = 0;
            focusTimer.start();
        }
    }

    //- Code -------------------------------------------------------
    function triggerFirstItem() {
        for (var row = 0; row < repeater.count; ++row) {
            var view = repeater.viewAt(row);
            if (!view.isEmpty()) {
                view.triggerFirstItem();
                return;
            }
        }
    }

    function focusedItem() {
        var row;
        for (row = 0; row < repeater.count; ++row) {
            var view = repeater.viewAt(row);
            var item = view.focusedItem();
            if (item) {
                return item;
            }
        }
        return null;
    }

    function updateFocus() {
        if (repeater.count == 0) {
            return;
        }
        var row;
        for (row = 0; row < repeater.count; ++row) {
            var view = repeater.viewAt(row);
            if (!view.isEmpty()) {
                view.focusFirstItem();
                return;
            }
        }
    }

    function handleTriggerResult(result) {
        if (result) {
            closeRequested();
        }
    }

    function updateRunning() {
        for (var idx = 0; idx < tabSourceModel.count; ++idx) {
            if (tabSourceModel.get(idx).model.running) {
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

    function connectModel(model) {
        if ("openSourceRequested" in model) {
            model.openSourceRequested.connect(main.openSourceRequested);
        }

        if ("query" in model) {
            // Model supports querying itself, bind the search criteria field to its "query" property
            queryBindingComponent.createObject(main, {"target": model});
        }

        if ("runningChanged" in model) {
            model.runningChanged.connect(main.updateRunning);
        }

        if ("applicationLaunched" in model) {
            model.applicationLaunched.connect(rootItem.applicationLaunched);
        }

        if ("addApp" in model) {
            rootItem.applicationLaunched.connect(model.addApp);
        }
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
        var view = component.createObject(parent, viewArgs);
        view.focusOtherViewRequested.connect(parent.navigate);
        return view;
    }

    function navigate(repeater, currentIdx, key, x) {
        function nextView() {
            for (var idx = currentIdx + 1; idx < repeater.count; ++idx) {
                var view = repeater.viewAt(idx);
                if (!view.isEmpty()) {
                    return view;
                }
            }
            return null;
        }

        function previousView() {
            for (var idx = currentIdx - 1; idx >= 0; --idx) {
                var view = repeater.viewAt(idx);
                if (!view.isEmpty()) {
                    return view;
                }
            }
            return null;
        }
        if (key == Qt.Key_Left) {
            var view = previousView();
            if (view) {
                view.focusLastItem();
                return true;
            }
        } else if (key == Qt.Key_Right) {
            var view = nextView();
            if (view) {
                view.focusFirstItem();
                return true;
            }
        } else if (key == Qt.Key_Up) {
            var view = previousView();
            if (view) {
                view.focusLastItemAtX(x);
                return true;
            }
        } else if (key == Qt.Key_Down) {
            var view = nextView();
            if (view) {
                view.focusFirstItemAtX(x);
                return true;
            }
        }
        return false;
    }

    function scrollToItem(item) {
        if (!item) {
            return;
        }
        var y = centralColumn.mapFromItem(item, 0, 0).y;
        if (y < centralFlickable.contentY) {
            centralFlickable.contentY = y;
        } else if (y + item.height > centralFlickable.contentY + centralFlickable.height) {
            centralFlickable.contentY = y + item.height - centralFlickable.height;
        }
    }

    function showMessage(icon, text) {
        messageItem.show(icon, text);
    }
}
