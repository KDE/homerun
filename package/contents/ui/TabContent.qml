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
    property string tabIconSource
    property string tabText
    property string searchCriteria
    property bool configureMode: false

    // Exposed by ourself
    property bool canGoBack: false
    property bool canGoForward: false
    property Item currentPage

    signal closeRequested
    signal updateTabOrderRequested
    signal setSearchFieldRequested(string text)
    signal sourcesUpdated(variant sources)

    //- Private ---------------------------------------------------
    Component {
        id: pageComponent
        Page {
            anchors.fill: parent
            configureMode: main.configureMode
            onSourcesUpdated: {
                main.sourcesUpdated(sources);
            }
            onCloseRequested: {
                main.closeRequested();
            }
            onOpenSourceRequested: {
                main.openSource(source);
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
        // navRow = back|previous + breadcrumbs
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

        Row {
            id: breadcrumbRow
            anchors {
                left: forwardButton.right
                leftMargin: 12
            }
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
                    property string source: model.source

                    flat: false
                    checked: model.index == currentPage.pathModel.count - 1
                    text: "› " + model.display
                    onClicked: {
                        if (!checked) {
                            openSource(model.source);
                        }
                    }
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
        if (activeFocus) {
            focusFirstView();
        }
    }

    onActiveFocusChanged: {
        if (activeFocus) {
            focusFirstView();
        }
    }

    function focusFirstView() {
        currentPage.focusFirstView();
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

    function openSource(source) {
        var page = createPage([source], { "showHeader": false });
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

    function createPage(sources, viewExtraArgs /*= {}*/) {
        var args = {
            sourceRegistry: sourceRegistry,
            sources: sources,
        };
        if (viewExtraArgs !== undefined) {
            for (var key in viewExtraArgs) {
                args[key] = viewExtraArgs[key];
            }
        }

        return pageComponent.createObject(pageContainer, args);
    }

    function reset() {
        TabContentInternal.goTo(0);
        TabContentInternal.clearHistoryAfterCurrentPage();
        searchCriteria = "";
    }
}
