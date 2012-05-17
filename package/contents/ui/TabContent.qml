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

import Qt 4.7
import org.kde.runnermodel 0.1 as RunnerModels
import org.kde.sal.components 0.1 as SalComponents
import org.kde.plasma.components 0.1 as PlasmaComponents

FocusScope {
    id: root
    property string modelName
    property variant modelArgs

    Component {
        id: serviceModelComponent
        SalComponents.SalServiceModel {
            path: "/"
        }
    }

    Component {
        id: runnerModelComponent
        RunnerModels.RunnerModel {
            query: searchField.text
        }
    }

    Component {
        id: placesModelComponent
        SalComponents.PlacesModel {
        }
    }

    PlasmaComponents.TextField {
        id: searchField

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        focus: true
        clearButtonShown: true
    }

    Flickable {
        id: resultsFlickable
        anchors {
            top: searchField.bottom
            topMargin: 12
            bottom: parent.bottom
            left: parent.left
            right: scrollBar.left
        }
        contentWidth: width
        contentHeight: resultsColumn.height
        clip: true
        Column {
            id: resultsColumn
            width: parent.width
            ResultsView {
                id: view
                width: parent.width

                onIndexClicked: {
                    model.run(index);
                }
            }
            Repeater {
                model: 10
                ResultsView {
                    model: view.model
                    width: parent.width

                    onIndexClicked: {
                        model.run(index);
                    }
                }
            }
        }
    }

    PlasmaComponents.ScrollBar {
        id: scrollBar
        flickableItem: resultsFlickable
        anchors {
            right: parent.right
            top: searchField.bottom
            bottom: parent.bottom
        }
    }

    Component.onCompleted: {
        if (modelName == "ServiceModel") {
            view.model = serviceModelComponent.createObject(view);
        } else if (modelName == "PlacesModel") {
            view.model = placesModelComponent.createObject(view);
        } else {
            view.model = runnerModelComponent.createObject(view);
            view.model.runners = modelArgs;
        }
    }
}
