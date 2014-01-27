/*
Copyright 2012 Aurélien Gâteau <agateau@kde.org>

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
// Self
#include <aboutdata.h>

// Local
#include <homerun_config.h>

// KDE
#include <KAboutData>

// Qt

namespace HomerunInternal
{

KAboutData *createAboutData()
{
    KAboutData *aboutData = new KAboutData(
        "homerun",
        // catalogName: we use the same catalog as the one loaded when homerun is used as a containment
        "plasma_applet_org.kde.homerun",
        // programName
        ki18nc("Program name", "Homerun"),
        HOMERUN_VERSION);
    aboutData->setLicense(KAboutData::License_GPL);
    aboutData->setCopyrightStatement(ki18n("(C) 2012-2014 Blue Systems"));
    aboutData->setProgramIconName("homerun");

    aboutData->addAuthor(ki18n("Shaun Reich"), ki18n("Developer"), "shaun.reich@blue-systems.com");
    aboutData->addAuthor(ki18n("Aurélien Gâteau"), ki18n("Developer"), "agateau@kde.org");
    aboutData->addAuthor(ki18n("Eike Hein"), ki18n("Developer"), "hein@kde.org");

    return aboutData;
}

}
