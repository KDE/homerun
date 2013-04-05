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
#include <QIcon>

// KDE
#include <KConfigGroup>
#include <KTemporaryFile>
#include <KDebug>
#include <qtest_kde.h>

#include <favoriteappsmodel.h>

using namespace Homerun;

QTEST_KDEMAIN(FavoriteAppsModelTest, GUI)

static KTemporaryFile *generateTestFile(const QString &content)
{
    KTemporaryFile *file = new KTemporaryFile;
    bool ok = file->open();
    Q_ASSERT(ok);
    file->write(content.toUtf8());
    file->flush();
    return file;
}

static void checkRole(QAbstractItemModel *model, int row, int role, const QVariant &expected)
{
    QModelIndex index = model->index(row, 0);
    QVERIFY(index.isValid());
    QVariant value = index.data(role);
    QCOMPARE(expected, value);
}

void FavoriteAppsModelTest::testLoad()
{
    QScopedPointer<KTemporaryFile> temp(generateTestFile(
        "[favorites][favorite-1]\n"
        "serviceId=kde4-konqbrowser.desktop\n"
        "[favorites][favorite-2]\n"
        "serviceId=kde4-dolphin.desktop\n"
        ));

    KSharedConfig::Ptr config = KSharedConfig::openConfig(temp->fileName());
    FavoriteAppsModel model;
    model.setConfig(config);

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

void FavoriteAppsModelTest::testAdd()
{
    QScopedPointer<KTemporaryFile> temp(generateTestFile(
        "[favorites][favorite-3]\n"
        "serviceId=kde4-konqbrowser.desktop\n"
        "[favorites][favorite-8]\n"
        "serviceId=kde4-dolphin.desktop\n"
        ));

    KSharedConfig::Ptr config = KSharedConfig::openConfig(temp->fileName());
    FavoriteAppsModel model;
    model.setConfig(config);

    model.addFavorite("app:kde4-konsole.desktop");

    // Check new favorite is in the model
    QCOMPARE(model.rowCount(), 3);
    checkRole(&model, 2, Qt::DisplayRole, "Konsole");
    checkRole(&model, 2, Qt::DecorationRole, "utilities-terminal");

    // Check config matches model
    FavoriteAppsModel model2;
    model2.setConfig(config);
    QCOMPARE(model2.rowCount(), 3);
    checkRole(&model2, 2, Qt::DisplayRole, "Konsole");
    checkRole(&model2, 2, Qt::DecorationRole, "utilities-terminal");
}

void FavoriteAppsModelTest::testAddEmpty()
{
    QScopedPointer<KTemporaryFile> temp(generateTestFile(""));

    KSharedConfig::Ptr config = KSharedConfig::openConfig(temp->fileName());
    FavoriteAppsModel model;
    model.setConfig(config);

    model.addFavorite("app:konsole.desktop");

    // Check new favorite is in the model
    QCOMPARE(model.rowCount(), 1);
    checkRole(&model, 0, Qt::DisplayRole, "Konsole");
    checkRole(&model, 0, Qt::DecorationRole, "utilities-terminal");

    // Check config matches model
    FavoriteAppsModel model2;
    model2.setConfig(config);
    QCOMPARE(model2.rowCount(), 1);
    checkRole(&model2, 0, Qt::DisplayRole, "Konsole");
    checkRole(&model2, 0, Qt::DecorationRole, "utilities-terminal");

}

void FavoriteAppsModelTest::testRemove()
{
    QScopedPointer<KTemporaryFile> temp(generateTestFile(
        "[favorites][favorite-4]\n"
        "serviceId=kde4-konqbrowser.desktop\n"
        "[favorites][favorite-8]\n"
        "serviceId=kde4-dolphin.desktop\n"
        "[favorites][favorite-9]\n"
        "serviceId=kde4-konsole.desktop\n"
        ));

    KSharedConfig::Ptr config = KSharedConfig::openConfig(temp->fileName());
    FavoriteAppsModel model;
    model.setConfig(config);

    // Drop Dolphin row
    model.removeFavorite("app:kde4-dolphin.desktop");

    // Check Dolphin has been removed from model
    QCOMPARE(model.rowCount(), 2);
    checkRole(&model, 1, Qt::DisplayRole, "Konsole");

    // Check config matches model
    FavoriteAppsModel model2;
    model2.setConfig(config);
    QCOMPARE(model2.rowCount(), 2);
    checkRole(&model2, 1, Qt::DisplayRole, "Konsole");
}

void FavoriteAppsModelTest::testMove()
{
    QModelIndex index;
    QScopedPointer<KTemporaryFile> temp(generateTestFile(
        "[favorites][favorite-4]\n"
        "serviceId=kde4-konqbrowser.desktop\n"
        "[favorites][favorite-8]\n"
        "serviceId=kde4-dolphin.desktop\n"
        "[favorites][favorite-9]\n"
        "serviceId=kde4-konsole.desktop\n"
        ));

    KSharedConfig::Ptr config = KSharedConfig::openConfig(temp->fileName());
    FavoriteAppsModel model;
    model.setConfig(config);

    // Move Dolphin after Konsole
    model.moveRow(1, 2);

    // Check model
    checkRole(&model, 1, Qt::DisplayRole, "Konsole");
    checkRole(&model, 2, Qt::DisplayRole, "Dolphin");
}

#include "favoriteappsmodeltest.moc"
