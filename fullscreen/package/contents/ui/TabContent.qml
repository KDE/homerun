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
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1 as QtExtra

import "TabContentInternal.js" as TabContentInternal

Item {
    id: main

    //- Public ----------------------------------------------------
    // Defined by outside world
    property Item rootItem
    property QtObject sourceRegistry
    property QtObject tabSourceModel
    property string tabIconSource
    property string tabText
    property string searchCriteria
    property bool configureMode: false
    property bool showActionListOverlay: false

    // Exposed by ourself
    property bool canGoBack: false
    property bool canGoForward: false
    property Item currentPage

    signal closeRequested
    signal updateTabOrderRequested
    signal setSearchFieldRequested(string text)

    //- Private ---------------------------------------------------
    Component {
        id: pageComponent
        Page {
            anchors.fill: parent
            rootItem: main.rootItem
            configureMode: main.configureMode
            showActionListOverlay: main.showActionListOverlay
            onCloseRequested: {
                main.closeRequested();
            }
            onOpenSourceRequested: {
                main.openSource(sourceId, sourceArguments);
            }
            onShowMessageRequested: {
                messageItem.show(icon, text);
            }
        }
    }

    // Ui
    SlidingContainer {
        id: editTabRow
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        show: configureMode

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 12

            PlasmaComponents.Label {
                text: i18n("Tab Icon:")
            }

            PlasmaComponents.ToolButton {
                flat: false
                iconSource: main.tabIconSource
                onClicked: {
                    var name = iconDialog.getIcon();
                    if (name != "") {
                        main.tabIconSource = name;
                    }
                }

                HomerunComponents.IconDialog {
                    id: iconDialog
                }
            }

            PlasmaComponents.Label {
                text: i18n("Tab Label:")
            }

            PlasmaComponents.TextField {
                text: main.tabText
                onTextChanged: main.tabText = text
            }
        }
    }

    SlidingContainer {
        // navRow = back|forward + breadcrumbs
        id: navRow
        property int maxHeight: 32
        show: canGoBack

        anchors {
            top: editTabRow.bottom
            left: parent.left
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

        PlasmaComponents.ButtonRow {
            id: breadcrumbRow
            anchors {
                left: forwardButton.right
                leftMargin: 12
            }
            spacing: 0
            height: navRow.maxHeight

            PlasmaComponents.ToolButton {
                height: breadcrumbRow.height
                flat: false
                iconSource: tabIconSource
                text: tabText
                onClicked: TabContentInternal.goTo(0);
            }

            Repeater {
                id: breadcrumbRepeater
                model: currentPage.pathModel
                delegate: PlasmaComponents.ToolButton {
                    height: breadcrumbRow.height

                    flat: false
                    checked: model.index == breadcrumbRepeater.count - 1
                    text: "› " + model.display
                    onClicked: {
                        // Do not use 'checked' here because by the time
                        // clicked() is emitted, 'checked' has already been set
                        // to true
                        if (model.index != breadcrumbRepeater.count - 1) {
                            openSource(model.sourceId, model.sourceArguments);
                        }
                    }
                }
            }
        }
    }

    PlasmaCore.SvgItem {
        id: hline
        anchors {
            left: parent.left
            right: parent.right
            bottom: navRow.bottom
            bottomMargin: -12
        }
        height: naturalSize.height
        z: 1000
        svg: PlasmaCore.Svg {
            imagePath: "widgets/scrollwidget"
        }
        elementId: "border-top"
    }

    Item {
        id: pageContainer
        anchors {
            left: parent.left
            leftMargin: 12
            top: hline.top
            right: parent.right
            bottom: parent.bottom
        }
    }

    MessageItem {
        id: messageItem
        anchors {
            top: navRow.show ? navRow.top : hline.bottom
            topMargin: navRow.show ? 0 : 6
            horizontalCenter: parent.horizontalCenter
        }
    }

    // Scripting
    Component.onCompleted: {
        var page = createPage(tabSourceModel);
        TabContentInternal.addPage(page);
        TabContentInternal.goToLastPage();
    }

    onActiveFocusChanged: {
        if (activeFocus) {
            currentPage.forceActiveFocus();
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
        var model = currentPage.pathModel;
        if (model.count >= 2) {
            // count - 1 is the breadcrumb for the current content
            // count - 2 is the breadcrumb for the content up
            var item = model.get(model.count - 2);
            openSource(item.sourceId, item.sourceArguments);
        } else {
            // "root" item is not in the path model, so we use a different way
            // to get to it
            TabContentInternal.goTo(0);
        }
    }

    Component {
        id: dynamicTabSourceModelComponent
        ListModel {
        }
    }

    function openSource(sourceId, sourceArguments) {
        // This tabSourceModel should look-like the C++ SourceModel used when
        // tab content is loaded from the config file.
        var tabSourceModel = dynamicTabSourceModelComponent.createObject(main);
        var page = createPage(tabSourceModel, { "showHeader": false });

        // Create the model for sourceId after creating the page so that the page
        // can be used as its parent, avoiding memory leaks.
        // See https://bugs.kde.org/show_bug.cgi?id=310217
        tabSourceModel.append({
            sourceId: sourceId,
            model: sourceRegistry.createModelFromArguments(sourceId, sourceArguments, page),
            configGroup: null
        })
        TabContentInternal.addPage(page);
        TabContentInternal.goToLastPage();
        page.forceActiveFocus();
    }

    function createPage(sourceModel, viewExtraArgs /*= {}*/) {
        var args = {
            sourceRegistry: sourceRegistry,
            tabSourceModel: sourceModel,
        };
        if (viewExtraArgs !== undefined) {
            for (var key in viewExtraArgs) {
                args[key] = viewExtraArgs[key];
            }
        }

        return pageComponent.createObject(pageContainer, args);
    }

    function triggerFirstItem() {
        currentPage.triggerFirstItem();
    }

    function reset() {
        TabContentInternal.goTo(0);
        TabContentInternal.clearHistoryAfterCurrentPage();
        searchCriteria = "";
    }
}
