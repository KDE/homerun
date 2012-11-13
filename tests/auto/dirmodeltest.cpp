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
#include "dirmodeltest.h"

// Local
#include <dirmodel.h>

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

using namespace Homerun;

QTEST_KDEMAIN(DirModelTest, GUI)

static void touch(const QString &path)
{
    QFile file(path);
    file.open(QIODevice::WriteOnly);
}

void DirModelTest::initTestCase()
{
    QString dir = KGlobal::dirs()->localxdgdatadir();
    QFile file(dir + "/user-places.xbel");
    file.remove();
}

void DirModelTest::testDirModelSortOrder()
{
    KTempDir tempDir("dirmodeltest");
    QDir dir(tempDir.name());
    QStringList dirNames = QStringList() << "aaa" << "Abc" << "Hello";
    QStringList fileNames = QStringList() << "foo" << "Goo" << "hoo" << "img1" << "img2" << "img10";

    Q_FOREACH(const QString &name, fileNames) {
        touch(dir.absoluteFilePath(name));
    }
    Q_FOREACH(const QString &name, dirNames) {
        dir.mkdir(name);
    }

    DirModel dirModel;

    QEventLoop loop;
    connect(dirModel.dirLister(), SIGNAL(completed()), &loop, SLOT(quit()));

    KUrl rootUrl = KUrl::fromLocalFile(dir.absolutePath());
    dirModel.init(rootUrl, rootUrl.fileName(), rootUrl);
    loop.exec();

    QStringList expected = dirNames + fileNames;
    for (int row = 0; row < dirModel.rowCount(); ++row) {
        QModelIndex index = dirModel.index(row, 0);
        QString name = index.data(Qt::DisplayRole).toString();
        QCOMPARE(name, expected[row]);
    }
}

void DirModelTest::testDirModelFavoriteId()
{
    KTempDir tempDir("dirmodeltest");
    QDir dir(tempDir.name());
    dir.mkdir("adir");
    touch(dir.absoluteFilePath("afile"));

    DirModel dirModel;
    QEventLoop loop;
    connect(dirModel.dirLister(), SIGNAL(completed()), &loop, SLOT(quit()));

    KUrl rootUrl = KUrl::fromLocalFile(dir.absolutePath());
    dirModel.init(rootUrl, rootUrl.fileName(), rootUrl);
    loop.exec();

    // Dir
    {
        QModelIndex index = dirModel.index(0, 0);
        QString favoriteId = index.data(DirModel::FavoriteIdRole).toString();
        QCOMPARE(favoriteId, QString("place:file://" + dir.absoluteFilePath("adir")));
    }
    // File
    {
        QModelIndex index = dirModel.index(1, 0);
        QString favoriteId = index.data(DirModel::FavoriteIdRole).toString();
        QCOMPARE(favoriteId, QString());
    }
}

void DirModelTest::testSortOrder()
{
    // Create sandbox
    KTempDir tempDir("dirmodeltest");
    QDir dir(tempDir.name());
    QStringList dirNames = QStringList() << "aaa" << "Abc" << "Hello";
    QStringList fileNames = QStringList() << "foo" << "Goo" << "hoo" << "img1" << "img2" << "img10";

    Q_FOREACH(const QString &name, fileNames) {
        touch(dir.absoluteFilePath(name));
    }
    Q_FOREACH(const QString &name, dirNames) {
        dir.mkdir(name);
    }

    // Point a DirModel to the sandbox
    DirModel dirModel;
    QEventLoop loop;
    connect(dirModel.dirLister(), SIGNAL(completed()), &loop, SLOT(quit()));

    KUrl rootUrl = KUrl::fromLocalFile(dir.absolutePath());
    dirModel.init(rootUrl, rootUrl.fileName(), rootUrl);
    loop.exec();

    // Check model content
    QStringList expected = dirNames + fileNames;
    QCOMPARE(dirModel.rowCount(), expected.length());
    for (int row = 0; row < dirModel.rowCount(); ++row) {
        QModelIndex index = dirModel.index(row, 0);
        QString name = index.data(Qt::DisplayRole).toString();
        QCOMPARE(name, expected[row]);
    }
}

#include <dirmodeltest.moc>
