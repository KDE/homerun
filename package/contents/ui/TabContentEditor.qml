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
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    id: main
    property QtObject sourceRegistry
    property variant sources
    property string searchCriteria

    signal sourcesUpdated(variant sources)

    // Components
    Component {
        id: sourceEditorComponent
        SourceEditor {
            id: sourceEditorMain
            sourceRegistry: main.sourceRegistry
            width: parent ? parent.width : 0

            sourceName: model.sourceName

            onRemoveRequested: {
                selectedSourcesModel.remove(model.index);
                main.updateSources();
            }
            onMoveRequested: {
                selectedSourcesModel.move(model.index, model.index + delta, 1);
                main.updateSources();
            }
        }
    }

    // Models
    ListModel {
        id: selectedSourcesModel
    }

    // Ui
    ListView {
        id: availableSourcesView
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
        width: 200

        model: sourceRegistry.availableSourcesModel()

        delegate: PlasmaComponents.Button {
            width: parent.width
            text: model.display
            onClicked: {
                selectedSourcesModel.append({sourceName: model.display});
                updateSources();
            }
        }
    }

    ListView {
        id: selectedSourcesView
        anchors {
            left: availableSourcesView.right
            leftMargin: 12
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
        model: selectedSourcesModel
        delegate: sourceEditorComponent
    }

    // Scripting
    Component.onCompleted: {
        fillSelectedSourcesModel();
    }

    function fillSelectedSourcesModel() {
        sources.forEach(function(sourceName) {
            selectedSourcesModel.append({sourceName: sourceName});
        });
    }

    function updateSources() {
        var lst = new Array();
        for (var idx = 0; idx < selectedSourcesModel.count; ++idx) {
            var item = selectedSourcesModel.get(idx);
            lst.push(item.sourceName);
        }
        sourcesUpdated(lst);
    }
}
