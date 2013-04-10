/*
Copyright 2012 Aurélien Gâteau <agateau@kde.org>

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
#include "favoriteappsmodeltest.h"

// Qt
#include <QDomNode>
#include <QDomElement>

// KDE
#include <KConfigGroup>
#include <KDebug>
#include <KStandardDirs>
#include <qtest_kde.h>

#include <favoriteappsmodel.h>

using namespace Homerun;

QTEST_KDEMAIN(FavoriteAppsModelTest, GUI)

static void checkRole(QAbstractItemModel *model, int row, int role, const QVariant &expected)
{
    QModelIndex index = model->index(row, 0);
    QVERIFY(index.isValid());
    QVariant value = index.data(role);
    QCOMPARE(expected, value);
}

void FavoriteAppsModelTest::writeTestConfig(const QString &content)
{
    QFile file(m_configFileName);
    bool ok = file.open(QIODevice::WriteOnly);
    QVERIFY2(ok, "Failed to open test config file for writing");
    file.write(content.toUtf8());
}

void FavoriteAppsModelTest::writeTestXml(const QString &content)
{
    QFile file(m_favoriteXmlFileName);
    bool ok = file.open(QIODevice::WriteOnly);
    QVERIFY2(ok, "Failed to open test xml file for writing");
    file.write(content.toUtf8());
}

void FavoriteAppsModelTest::init()
{
    m_favoriteXmlFileName = KStandardDirs::locateLocal("data", "homerun/favoriteapps.xml");
    m_configFileName = KStandardDirs::locateLocal("config", "homerunrc");

    // Start from a blank state
    QFile::remove(m_favoriteXmlFileName);
    QFile::remove(m_configFileName);
}

void FavoriteAppsModelTest::testLoad()
{
    writeTestXml(
        "<apps version='1'>\n"
        "<app serviceId='kde4-konqbrowser.desktop'/>\n"
        "<app serviceId='kde4-dolphin.desktop'/>\n"
        "</apps>");

    FavoriteAppsModel model;

    QCOMPARE(model.rowCount(), 2);

    // Test Konqueror row
    checkRole(&model, 0, Qt::DisplayRole, "Konqueror");
    checkRole(&model, 0, Qt::DecorationRole, "konqueror");
    checkRole(&model, 0, FavoriteAppsModel::FavoriteIdRole, "app:kde4-konqbrowser.desktop");

    // Test Dolphin row
    checkRole(&model, 1, Qt::DisplayRole, "Dolphin");
    checkRole(&model, 1, Qt::DecorationRole, "system-file-manager");
    checkRole(&model, 1, FavoriteAppsModel::FavoriteIdRole, "app:kde4-dolphin.desktop");
}

void FavoriteAppsModelTest::testImport()
{
    QVERIFY(!QFile::exists(m_favoriteXmlFileName));
    writeTestConfig(
        "[favorites][favorite-3]\n"
        "serviceId=kde4-konqbrowser.desktop\n"
        "[favorites][favorite-12]\n"
        "serviceId=kde4-konsole.desktop\n"
        "[favorites][favorite-8]\n"
        "serviceId=kde4-dolphin.desktop\n"
        "\n"
        "[Group1]\n"
        "foo=bar\n"
        "[Group2]\n"
        "foo=bar\n"
        );

    FavoriteAppsModel model;
    QCOMPARE(model.rowCount(), 3);
    // Order should be konq (3), dolphin (8), konsole (12)

    // Test Konqueror row
    checkRole(&model, 0, Qt::DisplayRole, "Konqueror");
    checkRole(&model, 0, Qt::DecorationRole, "konqueror");
    checkRole(&model, 0, FavoriteAppsModel::FavoriteIdRole, "app:kde4-konqbrowser.desktop");

    // Test Dolphin row
    checkRole(&model, 1, Qt::DisplayRole, "Dolphin");
    checkRole(&model, 1, Qt::DecorationRole, "system-file-manager");
    checkRole(&model, 1, FavoriteAppsModel::FavoriteIdRole, "app:kde4-dolphin.desktop");

    // Test Konsole row
    checkRole(&model, 2, Qt::DisplayRole, "Konsole");
    checkRole(&model, 2, Qt::DecorationRole, "utilities-terminal");
    checkRole(&model, 2, FavoriteAppsModel::FavoriteIdRole, "app:kde4-konsole.desktop");

    // Check favorites got removed from config file
    KSharedConfig::Ptr config = KSharedConfig::openConfig(m_configFileName, KConfig::SimpleConfig);
    QStringList list = config->groupList();
    list.sort();
    QCOMPARE(list.count(), 2);
    QCOMPARE(list[0], QString("Group1"));
    QCOMPARE(list[1], QString("Group2"));
}

void FavoriteAppsModelTest::testFirstLoad()
{
    QVERIFY(!QFile::exists(m_favoriteXmlFileName));
    QVERIFY(!QFile::exists(m_configFileName));

    FavoriteAppsModel model;
    // model should start with default apps, see data/favoriteapps.xml
    QCOMPARE(model.rowCount(), 3);
    checkRole(&model, 0, Qt::DisplayRole, "Konqueror");
    checkRole(&model, 1, Qt::DisplayRole, "KMail");
    checkRole(&model, 2, Qt::DisplayRole, "Dolphin");

    // we should not write the user xml file unless the user makes a change
    QVERIFY(!QFile::exists(m_favoriteXmlFileName));
}

void FavoriteAppsModelTest::testAdd()
{
    writeTestXml(
        "<apps version='1'>\n"
        "<app serviceId='kde4-konqbrowser.desktop'/>\n"
        "<app serviceId='kde4-dolphin.desktop'/>\n"
        "</apps>");

    FavoriteAppsModel model;

    model.addFavorite("app:kde4-konsole.desktop");

    // Check new favorite is in the model
    QCOMPARE(model.rowCount(), 3);
    checkRole(&model, 2, Qt::DisplayRole, "Konsole");
    checkRole(&model, 2, Qt::DecorationRole, "utilities-terminal");

    // Check changes got serialized
    FavoriteAppsModel model2;
    QCOMPARE(model2.rowCount(), 3);
    checkRole(&model2, 2, Qt::DisplayRole, "Konsole");
    checkRole(&model2, 2, Qt::DecorationRole, "utilities-terminal");
}

void FavoriteAppsModelTest::testAddToEmptyFavoriteList()
{
    writeTestXml("<apps version='1'></apps>");

    FavoriteAppsModel model;
    model.addFavorite("app:konsole.desktop");

    // Check new favorite is in the model
    QCOMPARE(model.rowCount(), 1);
    checkRole(&model, 0, Qt::DisplayRole, "Konsole");
    checkRole(&model, 0, Qt::DecorationRole, "utilities-terminal");

    // Check changes got serialized
    FavoriteAppsModel model2;
    QCOMPARE(model2.rowCount(), 1);
    checkRole(&model2, 0, Qt::DisplayRole, "Konsole");
    checkRole(&model2, 0, Qt::DecorationRole, "utilities-terminal");

}

void FavoriteAppsModelTest::testRemove()
{
    writeTestXml(
        "<apps version='1'>\n"
        "<app serviceId='kde4-konqbrowser.desktop'/>\n"
        "<app serviceId='kde4-dolphin.desktop'/>\n"
        "<app serviceId='kde4-konsole.desktop'/>\n"
        "</apps>");

    FavoriteAppsModel model;

    // Drop Dolphin row
    model.removeFavorite("app:kde4-dolphin.desktop");

    // Check Dolphin has been removed from model
    QCOMPARE(model.rowCount(), 2);
    checkRole(&model, 1, Qt::DisplayRole, "Konsole");

    // Check changes got serialized
    FavoriteAppsModel model2;
    QCOMPARE(model2.rowCount(), 2);
    checkRole(&model2, 1, Qt::DisplayRole, "Konsole");
}

void FavoriteAppsModelTest::testMove()
{
    QModelIndex index;
    writeTestXml(
        "<apps version='1'>\n"
        "<app serviceId='kde4-konqbrowser.desktop'/>\n"
        "<app serviceId='kde4-dolphin.desktop'/>\n"
        "<app serviceId='kde4-konsole.desktop'/>\n"
        "</apps>");

    FavoriteAppsModel model;

    // Move Dolphin after Konsole
    model.moveRow(1, 2);

    // Check model
    checkRole(&model, 1, Qt::DisplayRole, "Konsole");
    checkRole(&model, 2, Qt::DisplayRole, "Dolphin");

    // Check changes got serialized
    FavoriteAppsModel model2;
    checkRole(&model2, 1, Qt::DisplayRole, "Konsole");
    checkRole(&model2, 2, Qt::DisplayRole, "Dolphin");
}

#include "favoriteappsmodeltest.moc"
