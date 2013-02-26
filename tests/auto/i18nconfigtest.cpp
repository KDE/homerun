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
#include "i18nconfigtest.h"

// Local
// We include a .cpp. This is not a mistake. Content of this file is not really used.
#include <i18nconfig.cpp>

// KDE
#include <KConfig>
#include <KConfigGroup>
#include <qtest_kde.h>

// Qt

QTEST_KDEMAIN(I18nConfigTest, NoGUI)

void I18nConfigTest::test()
{
    KConfig config(QString(KDESRCDIR) + "/../../config/homerunrc");
    QStringList tabs = config.group("General").readEntry("tabs", QStringList());
    QVERIFY(!tabs.isEmpty());

    Q_FOREACH(const QString &tabGroupName, tabs) {
        KConfigGroup group = config.group(tabGroupName);
        QVERIFY(group.exists());
        QString name = group.readEntry("name");
        QVERIFY(!name.isEmpty());

        // Check name is marked for translation
        QVERIFY(sTabNames.contains(name));
    }
}

#include "i18nconfigtest.moc"
