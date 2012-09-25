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

Item {
    id: main
    property QtObject sourceRegistry
    property variant sources
    property string searchCriteria

    // Components
    Component {
        id: sourceEditorComponent
        SourceEditor {
            sourceRegistry: main.sourceRegistry
            width: parent.width
        }
    }

    // Ui
    Column {
        id: sourceEditorContainer
        anchors.fill: parent
    }

    // Scripting
    Component.onCompleted: {
        sources.forEach(createSourceEditor);
    }

    function createSourceEditor(sourceName) {
        sourceEditorComponent.createObject(sourceEditorContainer, {sourceName: sourceName});
    }
}
