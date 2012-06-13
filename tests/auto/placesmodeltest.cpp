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

void PlacesModelTest::testSortOrder()
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

    PlacesModel model;
    model.addPlace("Root", dir.absolutePath());
    bool foundRoot = false;
    for (int row = 0; row < model.rowCount(); ++row) {
        QModelIndex index = model.index(row, 0);
        QString name = index.data(Qt::DisplayRole).toString();
        if (name == "Root") {
            kWarning() << "trigger";
            model.trigger(row);
            foundRoot = true;
            break;
        }
    }
    QVERIFY2(foundRoot, "Could not find \"Root\" place entry");
    QTest::qWait(5000);

    QStringList expected = dirNames + fileNames;
    QCOMPARE(model.rowCount(), expected.length());
    for (int row = 0; row < model.rowCount(); ++row) {
        QModelIndex index = model.index(row, 0);
        QString name = index.data(Qt::DisplayRole).toString();
        QCOMPARE(name, expected[row]);
    }
}

#include <placesmodeltest.moc>
