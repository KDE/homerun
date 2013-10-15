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

    property int minimumWidth: frame.width + (theme.defaultFont.mSize.width * 18) + main.spacing + mainRow.anchors.leftMargin + mainRow.anchors.rightMargin
    property int minimumHeight: sourcesList.height + searchField.height + main.spacing + mainRow.anchors.topMargin + mainRow.anchors.bottomMargin

    property bool atTopEdge: (plasmoid.location == TopEdge)

    property int spacing: 6

    property string configFileName: "homerunkickerrc"
    property QtObject recentAppsModel
    property QtObject allAppsModel

    property bool useCustomButtonImage
    property string buttonImage

    property QtObject itemListDialogComponent: Qt.createComponent("ItemListDialog.qml");

    property Component compactRepresentation: PopupButton { id: button }

    function configChanged() {
        useCustomButtonImage = plasmoid.readConfig("useCustomButtonImage");
        buttonImage = urlConverter.convertToPath(plasmoid.readConfig("buttonImage"));
    }

    function showPopup(shown) {
        if (shown) {
            windowSystem.updateMargins(main);
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

    PlasmaCore.Svg {
        id: arrows

        imagePath: "widgets/arrows"
        size: "16x16"
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

    Item {
        anchors.fill: parent

        Row {
            id: mainRow

            anchors {
                fill: parent
                leftMargin: (windowSystem.margins.left == 0) ? 4 : 1;
                topMargin: (windowSystem.margins.top == 0) ? 4 : 1;
                rightMargin: (windowSystem.margins.right == 0) ? 4 : 1;
                bottomMargin: (windowSystem.margins.bottom == 0) ? 4 : 1;
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
                    height: lineSvg.elementSize("horizontal-line").height

                    svg: PlasmaCore.Svg {
                        id: lineSvg
                        imagePath: "widgets/line"
                    }

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

            Item {
                height: parent.height
                width: mainRow.width - frame.width - mainRow.spacing

                ItemListView {
                    id: sourcesList

                    height: (model == sourcesModel) ? model.count * itemHeight : height
                    width: parent.width

                    anchors {
                        top: atTopEdge ? searchField.bottom : undefined
                        topMargin: atTopEdge ? main.spacing : 0
                        bottom: atTopEdge ? undefined : searchField.top
                        bottomMargin: atTopEdge ? 0 : main.spacing
                    }

                    model: (searchField.text != "") ? allAppsModel : sourcesModel

                    Keys.onPressed: {
                        if (event.key == Qt.Key_Up && currentIndex == 0) {
                            searchField.focus = true;
                        } else if (event.key == Qt.Key_Down && currentIndex == sourcesModel.count - 1) {
                            searchField.focus = true;
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
                        if (event.key == Qt.Key_Up) {
                            sourcesList.currentIndex = sourcesModel.count - 1;
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
        }
    }

    Binding {
        target: plasmoid
        property: "passivePopup"
        value: sourcesList.containsMouse
    }

    HomerunKicker.SourceListFilterModel {
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
                var sourceFilter = model.model;
                var sourceName = sourceRegistry.visibleNameForSource(model.sourceId);

                if ("query" in model.model) {
                    queryBindingComponent.createObject(sourceDelegateMain, {"target": sourceFilter});
                } else {
                    sourceFilter = genericFilterComponent.createObject(model.model, {"sourceModel": model.model});
                }

                if ("modelForRow" in sourceFilter) {
                    var runner = (model.sourceId == "Runner") ? true : false;
                    multiModelExpander.createObject(sourceDelegateMain, {"display": sourceName,
                        "model": sourceFilter, "runner": runner});

                    if (model.sourceId == "FilterableInstalledApps") {
                        model.model.launched.connect(recentAppsModel.addApp);
                    }
                } else {
                    if (model.sourceId == "FavoriteApps") {
                        favoriteAppsRepeater.model = model.model;
                    } else {
                        if (model.sourceId == "Power") {
                            powerRepeater.model = model.model;
                        } else if (model.sourceId == "RecentApps") {
                            recentAppsModel = model.model;
                            model.model.addToDesktop.connect(appletProxy.addToDesktop);
                            model.model.addToPanel.connect(appletProxy.addToPanel);
                        }

                        sourcesModel.appendSource(sourceName, sourceFilter);
                    }
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
        id: multiModelExpander

        Repeater {
            width: 0
            height: 0

            visible: false

            property bool runner: false
            property string display

            delegate: Item { property string display: model.display }

            onItemAdded: {
                var mdl = model.modelForRow(index);

                if (runner) {
                    sourcesModel.appendSource(item.display, mdl);
                } else if (index == 0) {
                    allAppsModel = mdl;
                } else {
                    sourcesModel.insertSource(index, item.display, mdl);
                }

                mdl.addToDesktop.connect(appletProxy.addToDesktop);
                mdl.addToPanel.connect(appletProxy.addToPanel);
            }
        }
    }

    Component {
        id: genericFilterComponent

        HomerunFixes.SortFilterModel {
            filterRegExp: searchField.text
            property string name: sourceModel.name
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
                    console.log("moveRow(): source model cannot move rows. This method should not be called.");
                    return;
                }
                var sourceFrom = mapRowToSource(from);
                var sourceTo = mapRowToSource(to);
                sourceModel.moveRow(sourceFrom, sourceTo);
            }

            function setDesktopContainmentMutable(isMutable) {
                if ("setDesktopContainmentMutable" in sourceModel) {
                    sourceModel.setDesktopContainmentMutable(isMutable);
                }
            }

            function setAppletContainmentMutable(isMutable) {
                if ("setAppletContainmentMutable" in sourceModel) {
                    sourceModel.setAppletContainmentMutable(isMutable);
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
