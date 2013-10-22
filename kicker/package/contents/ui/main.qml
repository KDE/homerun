/*
 *   Copyright (C) 2013 by Eike Hein <hein@kde.org>
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

import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.draganddrop 1.0 as DragAndDrop
import org.kde.qtextracomponents 0.1 as QtExtra

import org.kde.homerun.components 0.1 as HomerunComponents
import org.kde.homerun.kicker 0.1 as HomerunKicker
import org.kde.homerun.fixes 0.1 as HomerunFixes

Item {
    id: main

    signal focusChanged()

    property int minimumWidth: frame.width + sourcesList.width + main.spacing + mainRow.anchors.leftMargin + mainRow.anchors.rightMargin
    property int maximumWidth: minimumWidth

    property int minimumHeight: preferredHeight
    property int preferredHeight: (sourcesModel.count * sourcesList.itemHeight) + searchField.height + main.spacing + mainRow.anchors.topMargin + mainRow.anchors.bottomMargin

    property bool atTopEdge: (plasmoid.location == TopEdge)

    property int spacing: 6

    property string configFileName: "homerunkickerrc"
    property QtObject recentAppsModel
    property QtObject allAppsModel
    property QtObject runnerModel

    property bool runnerSupport: true
    property bool useCustomButtonImage: false
    property string buttonImage: ""
    property bool alignToBottom: true

    property QtObject itemListDialogComponent: Qt.createComponent("ItemListDialog.qml");

    property Component compactRepresentation: PopupButton { id: button }

    function configChanged() {
        runnerSupport = plasmoid.readConfig("runnerSupport");
        useCustomButtonImage = plasmoid.readConfig("useCustomButtonImage");
        buttonImage = urlConverter.convertToPath(plasmoid.readConfig("buttonImage"));
        alignToBottom = plasmoid.readConfig("alignToBottom");
    }

    function showPopup(shown) {
        if (shown) {
            justOpenedTimer.start();
            searchField.focus = true;
        } else {
            sourcesList.currentIndex = -1;
            searchField.text = "";
        }
    }

    function favoriteModelForFavoriteId(favoriteId) {
        if (favoriteId === undefined || favoriteId === "") {
            return null;
        }
        var lst = favoriteId.split(":");
        if (lst.length === 0) {
            return null;
        }
        var model = sourceRegistry.favoriteModels[lst[0]];
        if (model === undefined) {
            return null;
        } else {
            return model;
        }
    }

    function handleFavoriteAction(actionId, actionArgument) {
        var favoriteId = actionArgument.favoriteId;
        var favoriteModel = favoriteModelForFavoriteId(favoriteId);
        if (actionId == "_homerun_favorite_remove") {
            favoriteModel.removeFavorite(favoriteId);
        } else if (actionId == "_homerun_favorite_add") {
            favoriteModel.addFavorite(favoriteId);
        }
    }

    Timer {
        id: justOpenedTimer

        repeat: false
        interval: 600
    }

    HomerunComponents.TabModel {
        id: tabModel
        configFileName: main.configFileName
        sourceRegistry: sourceRegistry
    }

    HomerunComponents.SourceRegistry {
        id: sourceRegistry
        configFileName: main.configFileName
    }

    HomerunKicker.AppletProxy {
        id: appletProxy
        item: plasmoid.action("configure")
    }

    HomerunKicker.UrlConverter {
        id: urlConverter
    }

    HomerunKicker.WindowSystem {
        id: windowSystem
    }

    PlasmaCore.FrameSvgItem {
        id : listItemSvg

        visible: false

        imagePath: "widgets/listitem"
        prefix: "normal"
    }

    PlasmaCore.Svg {
        id: arrows

        imagePath: "widgets/arrows"
        size: "16x16"
    }

    PlasmaCore.Svg {
        id: lineSvg
        imagePath: "widgets/line"

        property int horLineHeight: lineSvg.elementSize("horizontal-line").height
        property int vertLineWidth: lineSvg.elementSize("vertical-line").width
    }

    Item {
        anchors.fill: parent

        Row {
            id: mainRow

            anchors {
                fill: parent
                leftMargin: (windowSystem.margins.left == 0 || plasmoid.location == LeftEdge) ? 4 : 1;
                topMargin: (windowSystem.margins.top == 0 || plasmoid.location == TopEdge) ? 4 : 1;
                rightMargin: (windowSystem.margins.right == 0 || plasmoid.location == RightEdge) ? 4 : 1;
                bottomMargin: (windowSystem.margins.bottom == 0 || plasmoid.location == BottomEdge) ? 4 : 1;
            }

            spacing: main.spacing

            LayoutMirroring.enabled: (plasmoid.location == RightEdge) || (Qt.application.layoutDirection == Qt.RightToLeft)

            PlasmaCore.FrameSvgItem {
                id: frame

                width: theme.mediumIconSize + theme.smallIconSize
                height: parent.height

                imagePath: "widgets/listitem"
                prefix: "normal"

                DragAndDrop.DropArea {
                    id: favoritesDropArea

                    anchors.fill: favorites

                    onDragMove: {
                        if (favorites.animating) {
                            return;
                        }

                        var above = favorites.childAt(event.x, event.y);

                        if (above && above != event.mimeData.source) {
                            favoriteAppsRepeater.model.moveRow(event.mimeData.source.itemIndex, above.itemIndex);
                        }
                    }
                }

                Flow {
                    id: favorites

                    width: parent.width - theme.smallIconSize

                    anchors {
                        top: atTopEdge ? undefined : parent.top
                        topMargin: atTopEdge ? 0 : theme.smallIconSize / 2
                        bottom: atTopEdge ? parent.bottom : undefined
                        bottomMargin: atTopEdge ? theme.smallIconSize / 2 : 0
                        horizontalCenter: parent.horizontalCenter
                    }

                    property bool animating: false

                    move: Transition {
                        SequentialAnimation {
                            PropertyAction { target: favorites; property: "animating"; value: true }

                            NumberAnimation {
                                duration: 130
                                properties: "x, y"
                                easing.type: Easing.OutQuad
                            }

                            PropertyAction { target: favorites; property: "animating"; value: false }
                        }
                    }

                    spacing: main.spacing

                    Repeater {
                        id: favoriteAppsRepeater
                        delegate: SidebarItem { repeater: favoriteAppsRepeater }
                    }
                }

                PlasmaCore.SvgItem {
                    anchors {
                        top: atTopEdge ? power.bottom : undefined
                        topMargin: atTopEdge ? main.spacing : 0
                        bottom: atTopEdge ? undefined : power.top
                        bottomMargin: atTopEdge ? 0 : 6
                        horizontalCenter: parent.horizontalCenter
                    }

                    width: power.width
                    height: lineSvg.horLineHeight

                    svg: lineSvg
                    elementId: "horizontal-line"
                }

                Column {
                    id: power

                    anchors {
                        top: atTopEdge ? parent.top : undefined
                        topMargin: atTopEdge ? theme.smallIconSize / 2 : 0
                        bottom: atTopEdge ? undefined : parent.bottom
                        bottomMargin: atTopEdge ? 0 : theme.smallIconSize / 2
                        horizontalCenter: parent.horizontalCenter
                    }

                    width: parent.width - theme.smallIconSize

                    spacing: main.spacing

                    Repeater {
                        id: powerRepeater
                        delegate: SidebarItem { repeater: powerRepeater }
                    }
                }
            }

            Row {
                height: parent.height

                Item {
                    width: sourcesList.width
                    height: parent.height

                    ItemListView {
                        id: sourcesList

                        anchors {
                            top: atTopEdge ? searchField.bottom : undefined
                            topMargin: atTopEdge ? main.spacing : 0
                            bottom: atTopEdge ? undefined : searchField.top
                            bottomMargin: atTopEdge ? 0 : main.spacing
                        }

                        height: main.alignToBottom ? Math.min(model.count * itemHeight,
                            parent.height - searchField.height - anchors.topMargin - anchors.bottomMargin)
                            : parent.height - searchField.height - anchors.topMargin - anchors.bottomMargin

                        model: (searchField.text != "") ? allAppsModel : sourcesModel

                        KeyNavigation.up: searchField
                        KeyNavigation.down: searchField

                        Keys.onPressed: {
                            if (!runnerItemsColumns.count) {
                                return;
                            }

                            var target = null;

                            if (event.key == Qt.Key_Right) {
                                target = runnerItemsColumns.itemAt(0).listView;
                            } else if (event.key == Qt.Key_Left) {
                                target = runnerItemsColumns.itemAt(runnerItemsColumns.count - 1).listView;
                            }

                            if (target) {
                                currentIndex = -1;
                                target.currentIndex = 0;
                            }
                        }
                    }

                    HomerunFixes.TextField {
                        id: searchField

                        y: atTopEdge ? frame.y : (frame.y + frame.height) - height - 1

                        anchors {
                            horizontalCenter: parent.horizontalCenter
                        }

                        width: parent.width

                        clearButtonShown: true
                        placeholderText: i18n("Search...")

                        onFocusChanged: {
                            if (focus) {
                                sourcesList.currentIndex = -1;
                            }
                        }

                        onAccepted: {
                            if (allAppsModel && allAppsModel.count) {
                                allAppsModel.trigger(0, "", null);
                            }

                            plasmoid.hidePopup();
                        }

                        Keys.onPressed: {
                            if (!sourcesList.model.count) {
                                return;
                            }

                            if (event.key == Qt.Key_Up) {
                                sourcesList.currentIndex = Math.floor(sourcesList.height / sourcesList.itemHeight) - 1;
                            } else if (event.key == Qt.Key_Down) {
                                sourcesList.currentIndex = 0;
                            }
                        }

                        function appendText(newText) {
                            focus = true;
                            text = text + newText;
                        }
                    }
                }

                Repeater {
                    id: runnerItemsColumns

                    model: runnerSupport ? runnerModel : null

                    delegate: Item {
                        property Item listView: itemList

                        width: lineSvg.vertLineWidth + sourcesList.width + (main.spacing * 2)
                        height: mainRow.height

                        PlasmaCore.SvgItem {
                            id: vertLine

                            anchors {
                                left: parent.left
                                leftMargin: main.spacing
                            }

                            width: lineSvg.vertLineWidth
                            height: parent.height

                            svg: lineSvg
                            elementId: "vertical-line"
                        }

                        PlasmaCore.FrameSvgItem {
                            id: headerLabel

                            anchors {
                                left: vertLine.right
                                leftMargin: main.spacing
                            }

                            width: itemList.width
                            height: searchField.height

                            imagePath: "widgets/listitem"
                            prefix: "normal"

                            Text {
                                anchors.fill: parent
                                text: model.display

                                color: theme.textColor
                                elide: Text.ElideRight
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                                font.capitalization: theme.defaultFont.capitalization
                                font.family: theme.defaultFont.family
                                font.italic: theme.defaultFont.italic
                                font.letterSpacing: theme.defaultFont.letterSpacing
                                font.pointSize: theme.defaultFont.pointSize
                                font.strikeout: theme.defaultFont.strikeout
                                font.underline: theme.defaultFont.underline
                                font.weight: Font.Bold
                                font.wordSpacing: theme.defaultFont.wordSpacing
                            }
                        }

                        ItemListView {
                            id: itemList

                            anchors {
                                left: vertLine.right
                                leftMargin: main.spacing
                                top: atTopEdge ? headerLabel.bottom : undefined
                                topMargin: atTopEdge ? main.spacing : 0
                                bottom: atTopEdge ? undefined : parent.bottom
                                bottomMargin: atTopEdge ? 0 : Math.max(0, mainRow.height - sourcesList.y - sourcesList.height - itemHeight)
                            }

                            width: sourcesList.width
                            height: model ? (main.alignToBottom ?
                                Math.min(model.count * itemHeight, parent.height - headerLabel.height - anchors.topMargin - anchors.bottomMargin)
                                : parent.height - headerLabel.height - anchors.topMargin - anchors.bottomMargin) : 0

                            model: runnerItemsColumns.model.modelForRow(index);

                            Keys.onPressed: {
                                var target = null;

                                if (event.key == Qt.Key_Right) {
                                    if (index < (runnerItemsColumns.count - 1)) {
                                        target = runnerItemsColumns.itemAt(index + 1).listView;
                                    } else {
                                        target = sourcesList;
                                    }
                                } else if (event.key == Qt.Key_Left) {
                                    if (index == 0) {
                                        target = sourcesList;
                                    } else {
                                        target = runnerItemsColumns.itemAt(index - 1).listView;
                                    }
                                }

                                if (target) {
                                    currentIndex = -1;
                                    target.currentIndex = 0;
                                }
                            }
                        }
                    }

                    onItemAdded: {
                        main.minimumWidth = main.minimumWidth + (sourcesList.width + lineSvg.vertLineWidth + (main.spacing * 2));
                    }

                    onItemRemoved: {
                        main.minimumWidth = main.minimumWidth - (sourcesList.width + lineSvg.vertLineWidth + (main.spacing * 2));
                    }
                }
            }
        }
    }

    Binding {
        target: plasmoid
        property: "passivePopup"
        value: sourcesList.containsMouse
    }

    HomerunKicker.SourceListModel {
        id: sourcesModel
    }

    Repeater {
        model: tabModel
        delegate: Item {
            property QtObject sourceModel: model.sourceModel

            Repeater {
                model: sourceModel

                delegate: sourceDelegate
            }
        }
    }

    Component {
        id: sourceDelegate

        Item {
            id: sourceDelegateMain

            Component.onCompleted: {
                var sourceName = sourceRegistry.visibleNameForSource(model.sourceId);

                if ("modelForRow" in model.model) {
                    if (model.sourceId == "FilterableInstalledApps") {
                        queryBindingComponent.createObject(sourceDelegateMain, {"target":  model.model});
                        multiModelExpander.createObject(sourceDelegateMain, {"display": sourceName,
                            "model": model.model});
                        model.model.launched.connect(recentAppsModel.addApp);
                    } else if (model.sourceId == "Runner") {
                        runnerQueryBindingComponent.createObject(sourceDelegateMain, {"target":  model.model});
                        runnerModel = model.model;
                    }
                } else if (model.sourceId == "FavoriteApps") {
                    favoriteAppsRepeater.model = model.model;
                } else {
                    if (model.sourceId == "Power") {
                        powerRepeater.model = model.model;
                        return;
                    } else if (model.sourceId == "RecentApps") {
                        recentAppsModel = model.model;
                    }

                    sourcesModel.appendSource(sourceName, model.model);
                }
            }
        }
    }

    Component {
        id: queryBindingComponent
        Binding {
            property: "query"
            value: searchField.text
        }
    }

    Component {
        id: runnerQueryBindingComponent
        Binding {
            property: "query"
            value: searchField.text
            when: main.runnerSupport
        }
    }

    Component {
        id: multiModelExpander

        Repeater {
            width: 0
            height: 0

            visible: false

            property string display

            delegate: Item { property string display: model.display }

            onItemAdded: {
                var mdl = model.modelForRow(index);

                if (index == 0) {
                    allAppsModel = mdl;
                } else {
                    sourcesModel.insertSource(index, item.display, mdl);
                }
            }
        }
    }

    Component.onCompleted: {
        plasmoid.addEventListener ('ConfigChanged', configChanged);
        plasmoid.popupEvent.connect(showPopup);
        plasmoid.aspectRatioMode = IgnoreAspectRatio;

        var data = new Object;
        data["image"] = "homerun";
        data["mainText"] = i18n("Homerun Kicker");
        data["subText"] = i18n("Launch applications and manage your desktop.");
        plasmoid.popupIconToolTip = data;
    }
}
