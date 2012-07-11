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
#include "favoritemodeltest.h"

// Qt
#include <QIcon>

// KDE
#include <KConfigGroup>
#include <KTemporaryFile>
#include <KDebug>
#include <qtest_kde.h>

#include <favoritemodel.h>

QTEST_KDEMAIN(FavoriteModelTest, GUI)

static KTemporaryFile *generateTestFile(const QString &content)
{
    KTemporaryFile *file = new KTemporaryFile;
    Q_ASSERT(file->open());
    file->write(content.toUtf8());
    file->flush();
    return file;
}

void FavoriteModelTest::testLoad()
{
    QScopedPointer<KTemporaryFile> temp(generateTestFile(
        "[favorites][favorite-1]\n"
        "serviceId=kde4-konqbrowser.desktop\n"
        "[favorites][favorite-2]\n"
        "serviceId=kde4-dolphin.desktop\n"
        ));

    KSharedConfig::Ptr config = KSharedConfig::openConfig(temp->fileName());
    FavoriteModel model;
    model.setConfig(config);

    QCOMPARE(model.rowCount(), 2);

    // Test Konqueror row
    QModelIndex index = model.index(0, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("Konqueror"));
    QCOMPARE(index.data(Qt::DecorationRole).value<QIcon>().name(), QString("konqueror"));
    QCOMPARE(index.data(FavoriteModel::FavoriteIdRole).toString(), QString("app:kde4-konqbrowser.desktop"));

    // Test Dolphin row
    index = model.index(1, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("Dolphin"));
    QCOMPARE(index.data(Qt::DecorationRole).value<QIcon>().name(), QString("system-file-manager"));
    QCOMPARE(index.data(FavoriteModel::FavoriteIdRole).toString(), QString("app:kde4-dolphin.desktop"));
}

void FavoriteModelTest::testAdd()
{
    QModelIndex index;
    QScopedPointer<KTemporaryFile> temp(generateTestFile(
        "[favorites][favorite-3]\n"
        "serviceId=kde4-konqbrowser.desktop\n"
        "[favorites][favorite-8]\n"
        "serviceId=kde4-dolphin.desktop\n"
        ));

    KSharedConfig::Ptr config = KSharedConfig::openConfig(temp->fileName());
    FavoriteModel model;
    model.setConfig(config);

    model.addFavorite("app:kde4-konsole.desktop");

    // Check new favorite is in the model
    QCOMPARE(model.rowCount(), 3);
    index = model.index(2, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("Konsole"));
    QCOMPARE(index.data(Qt::DecorationRole).value<QIcon>().name(), QString("utilities-terminal"));

    // Check config matches model
    FavoriteModel model2;
    model2.setConfig(config);
    QCOMPARE(model2.rowCount(), 3);
    index = model2.index(2, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("Konsole"));
    QCOMPARE(index.data(Qt::DecorationRole).value<QIcon>().name(), QString("utilities-terminal"));
}

void FavoriteModelTest::testAddEmpty()
{
    QModelIndex index;
    QScopedPointer<KTemporaryFile> temp(generateTestFile(""));

    KSharedConfig::Ptr config = KSharedConfig::openConfig(temp->fileName());
    FavoriteModel model;
    model.setConfig(config);

    model.addFavorite("app:konsole.desktop");

    // Check new favorite is in the model
    QCOMPARE(model.rowCount(), 1);
    index = model.index(0, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("Konsole"));
    QCOMPARE(index.data(Qt::DecorationRole).value<QIcon>().name(), QString("utilities-terminal"));

    // Check config matches model
    FavoriteModel model2;
    model2.setConfig(config);
    QCOMPARE(model2.rowCount(), 1);
    index = model2.index(0, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("Konsole"));
    QCOMPARE(index.data(Qt::DecorationRole).value<QIcon>().name(), QString("utilities-terminal"));

}

void FavoriteModelTest::testRemove()
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
    FavoriteModel model;
    model.setConfig(config);

    // Drop Dolphin row
    model.removeFavorite("app:kde4-dolphin.desktop");

    // Check Dolphin has been removed from model
    QCOMPARE(model.rowCount(), 2);
    index = model.index(1, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("Konsole"));

    // Check config matches model
    FavoriteModel model2;
    model2.setConfig(config);
    QCOMPARE(model2.rowCount(), 2);
    index = model2.index(1, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("Konsole"));
}

#include "favoritemodeltest.moc"
