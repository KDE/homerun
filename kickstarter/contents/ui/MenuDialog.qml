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

import org.kde.homerun.fixes 0.1 as HomerunFixes

PlasmaCore.Dialog {
    id: menuDialog

    visible: false
    location: plasmoid.location
    windowFlags: Qt.Popup

    mainItem: Item {
        Row {
            id: mainRow

            x: 6
            y: 3
            height: parent.height - 6
            width: listRow.width + frame.width + spacing

            spacing: 6

            PlasmaCore.FrameSvgItem {
                width: 48
                height: parent.height
                id: frame

                imagePath: "widgets/listitem"
                prefix: "normal"

                Column {
                    id: favoritesColumn

                    anchors.top: parent.top
                    anchors.topMargin: 8
                    anchors.horizontalCenter: parent.horizontalCenter

                    spacing: 6

                    Repeater {
                        id: favoriteAppsRepeater
                        delegate: SidebarItem { repeater: favoriteAppsRepeater }
                    }
                }

                Column {
                    id: powerColumn

                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 8
                    anchors.horizontalCenter: parent.horizontalCenter

                    spacing: 6

                    PlasmaCore.SvgItem {
                        width: parent.width
                        height: lineSvg.elementSize("horizontal-line").height

                        svg: PlasmaCore.Svg {
                            id: lineSvg
                            imagePath: "widgets/line"
                        }

                        elementId: "horizontal-line"
                    }

                    Repeater {
                        id: powerRepeater
                        delegate: SidebarItem { repeater: powerRepeater }
                    }
                }
            }

            Column {
                spacing: 8

                height: parent.height

                HomerunFixes.TextField {
                    id: searchField

                    width: theme.defaultFont.mSize.width * 20

                    property QtObject model

                    clearButtonShown: true
                    placeholderText: i18n("Search...")

                    onTextChanged: {
                        if (text) {
                            resultList.model = null;
                            model.query = text;
                            sourceList.model = sourcesModel.get(0).sourceModel;
                            sourceList.expandable = false;
                        } else {
                            model.query = "";
                            sourceList.expandable = true;
                            sourceList.model = sourcesModel;
                        }
                    }

                    onAccepted: {
                        sourceList.model.trigger(sourceList.currentIndex, "", null);
                        menuDialog.visible = false;
                    }

                    Keys.onPressed: {
                        if (event.key == Qt.Key_Down) {
                            sourceList.focus = true;
                        } else if (event.key == Qt.Key_Right) {
                            resultList.focus = true;
                        }
                    }
                }

                Row {
                    id: listRow

                    spacing: 6

                    height: parent.height - searchField.height

                    ItemList {
                        id: sourceList

                        expandable: true
                        model: sourcesModel

                        onCurrentIndexChanged: {
                            if (model == sourcesModel && currentIndex >= 0) {
                                resultList.model = sourcesModel.get(currentIndex).sourceModel;
                            }
                        }

                        Keys.onPressed: {
                            if (event.key == Qt.Key_Right) {
                                resultList.focus = true;
                            }
                        }
                    }

                    ItemList {
                        id: resultList

                        Keys.onPressed: {
                            if (event.key == Qt.Key_Left) {
                                sourceList.focus = true;
                            }
                        }
                    }
                }
            }
        }
    }

    ListModel {
        id: sourcesModel
    }

    Repeater {
        model: tabModel
        delegate: Item {
            property QtObject sourceModel: model.sourceModel

                Repeater {
                    model: sourceModel

                    delegate: Item {
                        property QtObject modelProxy: model
                    }

                    onItemAdded: {
                        if (item.modelProxy.sourceId == "FilterableInstalledApps") {
                            for (var i = 0; i < 1; ++i) {
                                sourcesModel.append({ "display": item.modelProxy.model.nameForRow(i),
                                                      "sourceModel": item.modelProxy.model.modelForRow(i) });
                            }

                            searchField.model = item.modelProxy.model;
                        } else {
                            if (item.modelProxy.sourceId == "FavoriteApps") {
                                favoriteAppsRepeater.model = item.modelProxy.model;
                            }

                            if (item.modelProxy.sourceId == "Power") {
                                powerRepeater.model = item.modelProxy.model;
                            }

                            sourcesModel.append({ "display": sourceRegistry.visibleNameForSource(item.modelProxy.sourceId),
                                                  "sourceModel": item.modelProxy.model });
                        }
                    }
                }
            }
        }

    onVisibleChanged: {
        if (visible) {
            searchField.focus = true;
            sourceList.currentIndex = 0;
            mainItem.width = mainRow.width + mainRow.x
            mainItem.height = Math.max(500, favoritesColumn.height + favoritesColumn.anchors.topMargin
                + powerColumn.height + powerColumn.anchors.bottomMargin + favoritesColumn.spacing + (mainRow.y * 2))
            updatePosition();
        } else {
            searchField.text = "";
            sourceList.model = sourcesModel;
            sourceList.currentIndex = 0;
        }
    }

    onHeightChanged: updatePosition()

    function updatePosition() {
        var pos = popupPosition(main, Qt.AlignCenter);
        x = pos.x;
        y = pos.y;
    }
}