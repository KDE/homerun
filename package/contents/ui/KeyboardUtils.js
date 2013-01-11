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

function setTabOrder(lst) {
    var idx;
    var lastIdx = lst.length - 1;
    for (idx = 0; idx <= lastIdx; ++idx) {
        var item = lst[idx];
        if (idx > 0) {
            item.KeyNavigation.backtab = lst[idx - 1];
        }
        if (idx < lastIdx) {
            item.KeyNavigation.tab = lst[idx + 1];
        }
    }
}

/**
 * Look for a callback to call for a key event.
 * Marks the event as accepted if a callback was found.
 * @param type:list<variant> lst a list of the form: [ [modifier, key, callback], [modifier, key, callback]... ].
 *            Key can be either a Qt key code, for example Qt.Key_F1, or a one-char string,
 *            for example "y".
 * @param type:event event the key event.
 */
function processShortcutList(lst, event) {
    function eventMatchesKey(event, key) {
        var keyType = typeof(key);
        if (keyType === "string") {
            return event.text === key;
        } else if (keyType === "number") {
            return event.key === key;
        } else {
            console.log("KeyboardUtils.js:processShortcutList: Key " + key + " is of wrong type");
            return false;
        }
    }
    event.accepted = lst.some(function(x) {
        var modifiers = x[0];
        var key = x[1];
        var callback = x[2];
        if (modifiers !== null && event.modifiers !== modifiers) {
            return false;
        }
        if (eventMatchesKey(event, key)) {
            callback();
            return true;
        }
        return false;
    });
}
