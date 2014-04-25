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

import org.kde.homerun.components 0.1 as HomerunComponents
import org.kde.homerun.kicker 0.1 as HomerunKicker
import org.kde.homerun.fixes 0.1 as HomerunFixes

Item {
    id: main

    signal focusChanged()

    property int minimumWidth: mainRow.implicitWidth + mainRow.anchors.leftMargin + mainRow.anchors.rightMargin
    property int maximumWidth: minimumWidth
    property int preferredWidth: minimumWidth

    property int minimumHeight: preferredHeight
    property int maximumHeight: windowSystem.workArea().height
    property int preferredHeight: (sourcesModel.count * sourcesList.itemHeight) + searchField.height + main.spacing + mainRow.anchors.topMargin + mainRow.anchors.bottomMargin

    property variant location: plasmoid.location // HACK: plasmoid.location doesn't work as a notifyable
                                                 // property for some reason. We attach to locationChanged
                                                 // in Component.onCompleted.
    property bool atTopEdge: (location == TopEdge)

    property int spacing: 6

    property string configFileName: "homerunkickerrc"
    property QtObject recentAppsModel
    property QtObject allAppsModel
    property QtObject runnerModel

    property string icon: plasmoid.readConfig("icon")
    property bool runnerSupport: true
    property bool useCustomButtonImage: false
    property string buttonImage: ""
    property bool alignToBottom: true
    property int nameFormat: 0

    property QtObject itemListDialogComponent: Qt.createComponent("ItemListDialog.qml");

    property Component compactRepresentation: PopupButton { id: button }

    function configChanged() {
        icon = plasmoid.readConfig("icon");
        runnerSupport = plasmoid.readConfig("runnerSupport");
        useCustomButtonImage = plasmoid.readConfig("useCustomButtonImage");
        buttonImage = urlConverter.convertToPath(plasmoid.readConfig("buttonImage"));
        alignToBottom = plasmoid.readConfig("alignToBottom");
        nameFormat = plasmoid.readConfig("nameFormat");
    }

    function updateLocation() {
        location = plasmoid.location;
    }

    function updateSvgMetrics() {
        lineSvg.horLineHeight = lineSvg.elementSize("horizontal-line").height;
        lineSvg.vertLineWidth = lineSvg.elementSize("vertical-line").width;
    }

    function showPopup(shown) {
        if (shown) {
            maximumHeight = windowSystem.workArea().height;
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

    function action_menuedit() {
        processRunner.execute("kmenuedit");
    }

    Timer {
        id: justOpenedTimer

        repeat: false
        interval: 600
    }

    HomerunKicker.AppletProxy {
        id: appletProxy
        item: plasmoid.action("configure")
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

    HomerunKicker.ProcessRunner {
        id: processRunner
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

        property int horLineHeight
        property int vertLineWidth
    }

    Item {
        anchors.fill: parent

        Row {
            id: mainRow

            anchors {
                fill: parent
                leftMargin: 4;
                topMargin: (location == TopEdge) ? 3 : 1;
                rightMargin: (location == RightEdge) ? 3 : 1;
                bottomMargin: (location == BottomEdge) ? 3 : 1;
            }

            spacing: main.spacing

            LayoutMirroring.enabled: (location == RightEdge) || (Qt.application.layoutDirection == Qt.RightToLeft)

            PlasmaCore.FrameSvgItem {
                id: frame

                width: theme.mediumIconSize + theme.smallIconSize
                height: parent.height

                imagePath: "widgets/listitem"
                prefix: "normal"

                SidebarSection {
                    id: favoriteApps

                    x: theme.smallIconSize / 2
                    y: atTopEdge ? sidebarSeparator.y + sidebarSeparator.height + main.spacing : theme.smallIconSize / 2

                    width: parent.width - theme.smallIconSize
                    height: (parent.height - sidebarSeparator.height - powerSessionFavorites.height
                        - (2 * main.spacing) - (2 * (theme.smallIconSize / 2)))
                }

                PlasmaCore.SvgItem {
                    id: sidebarSeparator

                    x: theme.smallIconSize / 2
                    y: ((atTopEdge ? powerSessionFavorites.y + powerSessionFavorites.height : favoriteApps.y
                        + favoriteApps.height) + main.spacing)

                    width: parent.width - theme.smallIconSize
                    height: lineSvg.horLineHeight

                    svg: lineSvg
                    elementId: "horizontal-line"
                }

                SidebarSection {
                    id: powerSessionFavorites

                    x: theme.smallIconSize / 2
                    y: atTopEdge ? theme.smallIconSize / 2 : sidebarSeparator.y + sidebarSeparator.height + main.spacing

                    width: parent.width - theme.smallIconSize
                    height: (model.count * width) + ((model.count - 1) * main.spacing)
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

                        width: theme.defaultFont.mSize.width * 18
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
                        model.model.applicationLaunched.connect(recentAppsModel.addApp);
                    } else if (model.sourceId == "Runner") {
                        runnerQueryBindingComponent.createObject(sourceDelegateMain, {"target":  model.model});
                        runnerModel = model.model;
                    }
                } else if (model.sourceId == "FavoriteApps") {
                    favoriteApps.model = model.model;
                } else {
                    if (model.sourceId == "CombinedPowerSession") {
                        model.model.showFavoritesActions = true;
                        powerSessionFavorites.model = model.model.favoritesModel();
                    } else if (model.sourceId == "RecentApps") {
                        recentAppsModel = model.model;
                    }

                    // FIXME: KPluginInfo::name() doesn't load the translation from the .desktop
                    // file for some reason, probably due to yet another i18n bug in KConfig (bad
                    // memories of debugging a similar problem with dfaure once ...). Let's paper
                    // over it for release by falling back to the gettext catalog.
                    if (model.sourceId == "RecentDocuments") {
                        sourceName = i18n("Recent Documents");
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
        plasmoid.addEventListener('ConfigChanged', configChanged);
        plasmoid.popupEvent.connect(showPopup);
        plasmoid.aspectRatioMode = IgnoreAspectRatio;
        updateSvgMetrics();
        theme.themeChanged.connect(updateSvgMetrics);

        // HACK: See location prop.
        plasmoid.locationChanged.connect(updateLocation);

        var data = new Object;
        data["image"] = "homerun";
        data["mainText"] = i18n("Homerun Kicker");
        data["subText"] = i18n("Launch applications and manage your desktop.");
        plasmoid.popupIconToolTip = data;

        plasmoid.setAction("menuedit", i18n("Edit Applications..."));
    }
}
