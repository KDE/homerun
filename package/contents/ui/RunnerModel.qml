/*
 * Copyright 2012 Aurélien Gâteau <agateau@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 1.1

import org.kde.runnermodel 0.1 as RunnerModels

/**
 * Inherits from Plasma RunnerModel to add SAL-related features
 */
RunnerModels.RunnerModel {
    property string name
    property QtObject favoriteModel

    function favoriteIcon(obj) {
        return obj.runnerid == "services" ? "bookmarks" : ""
    }

    function triggerFavoriteAction(obj) {
        if (obj.runnerid != "services") {
            return;
        }
        favoriteModel.append(obj.data);
    }
}