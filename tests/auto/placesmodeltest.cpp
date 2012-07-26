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
#include "placesmodeltest.h"

// Local
#include <placesmodel.h>

// Qt
#include <QDir>
#include <QEventLoop>
#include <QFile>

// KDE
#include <KDebug>
#include <KDirLister>
#include <KGlobal>
#include <KStandardDirs>
#include <KTempDir>
#include <qtest_kde.h>

QTEST_KDEMAIN(PlacesModelTest, GUI)

static void touch(const QString &path)
{
    QFile file(path);
    file.open(QIODevice::WriteOnly);
}

void PlacesModelTest::initTestCase()
{
    QString dir = KGlobal::dirs()->localxdgdatadir();
    QFile file(dir + "/user-places.xbel");
    file.remove();
}

void PlacesModelTest::testProxyDirModelSortOrder()
{
    KTempDir tempDir("placesmodeltest");
    QDir dir(tempDir.name());
    QStringList dirNames = QStringList() << "aaa" << "Abc" << "Hello";
    QStringList fileNames = QStringList() << "foo" << "Goo" << "hoo" << "img1" << "img2" << "img10";

    Q_FOREACH(const QString &name, fileNames) {
        touch(dir.absoluteFilePath(name));
    }
    Q_FOREACH(const QString &name, dirNames) {
        dir.mkdir(name);
    }

    ProxyDirModel proxyDirModel;

    QEventLoop loop;
    connect(proxyDirModel.dirLister(), SIGNAL(completed()), &loop, SLOT(quit()));
    proxyDirModel.dirLister()->openUrl(dir.absolutePath());
    loop.exec();

    QStringList expected = dirNames + fileNames;
    for (int row = 0; row < proxyDirModel.rowCount(); ++row) {
        QModelIndex index = proxyDirModel.index(row, 0);
        QString name = index.data(Qt::DisplayRole).toString();
        QCOMPARE(name, expected[row]);
    }
}

void PlacesModelTest::testProxyDirModelFavoriteId()
{
    KTempDir tempDir("placesmodeltest");
    QDir dir(tempDir.name());
    dir.mkdir("adir");
    touch(dir.absoluteFilePath("afile"));

    ProxyDirModel proxyDirModel;

    QEventLoop loop;
    connect(proxyDirModel.dirLister(), SIGNAL(completed()), &loop, SLOT(quit()));
    proxyDirModel.dirLister()->openUrl(dir.absolutePath());
    loop.exec();

    // Dir
    {
        QModelIndex index = proxyDirModel.index(0, 0);
        QString favoriteId = index.data(ProxyDirModel::FavoriteIdRole).toString();
        QCOMPARE(favoriteId, QString("place:file://" + dir.absoluteFilePath("adir")));
    }
    // File
    {
        QModelIndex index = proxyDirModel.index(1, 0);
        QString favoriteId = index.data(ProxyDirModel::FavoriteIdRole).toString();
        QCOMPARE(favoriteId, QString());
    }
}

void PlacesModelTest::testSortOrder()
{
    // Create sandbox
    KTempDir tempDir("placesmodeltest");
    QDir dir(tempDir.name());
    QStringList dirNames = QStringList() << "aaa" << "Abc" << "Hello";
    QStringList fileNames = QStringList() << "foo" << "Goo" << "hoo" << "img1" << "img2" << "img10";

    Q_FOREACH(const QString &name, fileNames) {
        touch(dir.absoluteFilePath(name));
    }
    Q_FOREACH(const QString &name, dirNames) {
        dir.mkdir(name);
    }

    // Point a PlacesModel to the sandbox
    FavoritePlacesModel rootModel;
    PlacesModel model;
    model.setRootModel(&rootModel);
    model.setArguments("rootUrl=file:///,rootName=Root,url=file://" + dir.absolutePath());
    QTest::qWait(5000);

    // Check model content
    QStringList expected = dirNames + fileNames;
    QCOMPARE(model.rowCount(), expected.length());
    for (int row = 0; row < model.rowCount(); ++row) {
        QModelIndex index = model.index(row, 0);
        QString name = index.data(Qt::DisplayRole).toString();
        QCOMPARE(name, expected[row]);
    }
}

#include <placesmodeltest.moc>
