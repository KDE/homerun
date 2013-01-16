/*
Copyright 2013 Aurélien Gâteau <agateau@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
import QtQuick 1.1
import org.kde.homerun.components 0.1 as HomerunComponents
import org.kde.homerun.fixes 0.1 as HomerunFixes

/**
 * A label which uses Plasma colors by default, but appears differently when
 * run as a containment: it shows a shadow behind itself and both shadow and
 * text colors can be customized.
 */
HomerunFixes.Label {
    id: main

    /**
     * Whether a shadow should be used in containment mode.
     * Set this property to false to always disable the shadow.
     */
    property bool useShadow: true

    Component {
        id: shadowComponent
        HomerunComponents.ShadowEffect {
            xOffset: 0
            yOffset: 1
            blurRadius: 3
            enabled: main.useShadow
        }
    }

    Component.onCompleted: {
        var isContainment = "plasmoid" in this;
        if (isContainment) {
            main.effect = shadowComponent.createObject(main);
            readConfig();
            plasmoid.addEventListener("ConfigChanged", readConfig);
        } else {
            main.color = theme.textColor;
        }
    }

    function readConfig() {
        main.effect.color = plasmoid.readConfig("shadowColor");
        main.effect.blurRadius = plasmoid.readConfig("shadowBlurRadius");
        main.color = plasmoid.readConfig("textColor");
    }
}
