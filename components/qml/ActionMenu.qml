/*
Copyright 2013 Aurélien Gâteau <agateau@kde.org>

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

import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    id: main
    /// Public

    /// type:list<map<string,variant>> list of actions
    property variant actionList

    property Item visualParent

    property bool opened: menu ? (menu.status != PlasmaComponents.DialogStatus.Closed) : false

    signal actionClicked(string actionId, variant actionArgument)

    function open(x, y) {
        if (x && y) {
            menu.open(x, y);
        } else {
            menu.open();
        }
    }

    /// Internal
    property QtObject menu

    Component {
        id: contextMenuComponent
        PlasmaComponents.ContextMenu {
            visualParent: main.visualParent
        }
    }

    Component {
        id: contextMenuItemComponent

        PlasmaComponents.MenuItem {
            property variant actionItem

            text: actionItem.text ? actionItem.text : ""
            enabled: actionItem.type != "title" && ("enabled" in actionItem ? actionItem.enabled : true)
            separator: actionItem.type == "separator"
            icon: actionItem.icon ? actionItem.icon : null

            onClicked: {
                actionClicked(actionItem.actionId, actionItem.actionArgument);
            }
        }
    }

    Component {
        id: emptyMenuItemComponent
        PlasmaComponents.MenuItem {
            text: i18n("(Empty)")
            enabled: false
        }
    }

    // Code
    onActionListChanged: refreshMenu();

    function refreshMenu() {
        if (menu) {
            menu.destroy();
        }

        menu = contextMenuComponent.createObject(main);

        if (!actionList || actionList.length == 0) {
            var item = emptyMenuItemComponent.createObject(menu);
            menu.addMenuItem(item);
            return;
        }

        actionList.forEach(function(actionItem) {
            var item = contextMenuItemComponent.createObject(menu, {
                "actionItem": actionItem,
            });
            menu.addMenuItem(item);
        });
    }
}
