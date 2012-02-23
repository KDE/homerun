
/*****************************************************************************
 *   Copyright (C) 2011, 2012 by Shaun Reich <shaun.reich@kdemail.net>        *
 *                                                                            *
 *   This program is free software; you can redistribute it and/or            *
 *   modify it under the terms of the GNU General Public License as           *
 *   published by the Free Software Foundation; either version 2 of           *
 *   the License, or (at your option) any later version.                      *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *****************************************************************************/

import QtQuick 1.1
import org.kde.qtextracomponents 0.1

import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.components 0.1

Item {
    id: tasks

    property int minimumWidth: 300
    property int minimumHeight: 300

//    PlasmaCore.DataSource {
//        id: tasksSource
//        engine: "tasks"
//        onSourceAdded: connectSource(source)
//        onSourceRemoved: disconnectSource(source)
//
//        Component.onCompleted: connectedSources = sources
//    }
//
//    PlasmaCore.DataModel {
//        id: tasksModel
//        dataSource: tasksSource
//    }
//
    Component.onCompleted: {
//        plasmoid.popupIcon = "utilities-terminal";
 //       plasmoid.aspectRatioMode = IgnoreAspectRatio;
    }

    //                //FIXME: make it not be created in the first place..
    //            //            visible: model.onCurrentDesktop
    //
    //            GridView.onRemove: SequentialAnimation {
    //                PropertyAction { target: wrapper ; property: "GridView.delayRemove"; value: true }
    //                NumberAnimation { target: taskBackground; property: "opacity"; to: 0; duration: 2500; easing.type: Easing.InOutQuad }
    //                PropertyAction { target: wrapper; property: "GridView.delayRemove"; value: false }
    //            }
    //

    //

    //        }

    Flow {
        id: flower
        anchors.fill: parent

        move: Transition {
            NumberAnimation {
                properties: "x,y"
                easing {type: Easing.OutBack}
            }
        }

        Repeater {
            id: repeater
            model: tasksModel

            Item {
                id: wrapper
                width: taskBackground.width
                height: taskBackground.height

                states: [

                    State {
                        name: "none"
                        when: !hovered && !model.Minimized

                        PropertyChanges {
                            target: taskBackground
                            prefix: "normal"
                        }
                    },
                    State {
                        name: "hovered"
                        when: hovered && !model.Minimized

                        PropertyChanges {
                            target: taskBackground
                            prefix: "hover"
                        }
                    },
                    State {
                        name: "minimized"
                        when: model.Minimized

                        PropertyChanges {
                            target: taskBackground
                            prefix: "focus"
                        }
                    }
                ]

                property bool hovered: false
                property ContextMenu contextMenu
                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.RightButton

                    onClicked: {
 //                       for (i in model) {
//                            print(i + " " + model[i])
  //                      }

                        // if (!contextMenu) {
                        if (mouse.button == Qt.RightButton) {
                            contextMenu = contextMenuComponent.createObject(taskBackground)
                            contextMenu.open()
                        }
                    }

                    onEntered: {
                        hovered = true;
                    }

                    onExited: {
                        hovered = false;
                    }
                }

                PlasmaCore.FrameSvgItem {
                    id: taskBackground

//                    width: text.width + icon.width

                    anchors {
                        left: icon.left
                        right: text.right
                        top: parent.top
                        bottom: text.bottom

                        leftMargin: -8
                        rightMargin: -8
                        topMargin: -8
                        bottomMargin: -8
                    }

                    imagePath: "widgets/tasks"
//                    prefix: model.Minimized ? "normal" : ""
                }

                QIconItem {
                    id: icon

                    anchors { left: parent.left; verticalCenter: taskBackground.verticalCenter }

                    icon: model.DecorationRole
                    width: 22
                    height: 22
                }

                PlasmaComponents.Label {
                    id: text

                    anchors { left: icon.right; top: icon.top; bottom: icon.bottom }

                    height: 20
                    width: 200

                    verticalAlignment: Text.AlignVCenter

                    clip: true
                    text: model.DisplayRole
                }

                Component {
                    id: contextMenuComponent
                    PlasmaComponents.ContextMenu {
                        PlasmaComponents.MenuItem {
                            //text: model.DisplayRole
                            text: "Close"
                        }

                        PlasmaComponents.MenuItem {
                            text: "More Actions"

                        }

                        PlasmaComponents.MenuItem {
                            text: "Task Manager Settings"
                        }

                        PlasmaComponents.MenuItem {
                            text: "Show A Launcher When Not Running"
                        }

                        PlasmaComponents.MenuItem {
                            text: "Start New Instance"
                        }

                        PlasmaComponents.MenuItem {
                            text: "Minimize"
                        }

                        PlasmaComponents.MenuItem {
                            text: "Maximize"
                        }
                    }
                }
            }
        }
    }

    
}




//        PlasmaComponents.Label {
//            id: header
//            text: i18n("Konsole Profiles")
//            anchors { horizontalCenter: parent.horizontalCenter }
//            horizontalAlignment: Text.AlignHCenter
//        }
//

//
//    Text {
//        id: textMetric
//        visible: false
//        // translated but not used, we just need length/height
//        text: i18n("Arbitrary String Which Says The Something")
//    }
//
