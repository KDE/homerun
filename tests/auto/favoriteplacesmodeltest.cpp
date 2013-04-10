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
// Self
#include <favoriteplacesmodeltest.h>

// Local
#include <favoriteplacesmodel.h>

// KDE
#include <KDebug>
#include <KGlobal>
#include <KStandardDirs>
#include <qtest_kde.h>

// Qt
#include <QFile>

using namespace Homerun;

QTEST_KDEMAIN(FavoritePlacesModelTest, NoGUI)

static void checkRole(QAbstractItemModel *model, int row, int role, const QVariant &expected)
{
    QModelIndex index = model->index(row, 0);
    QVERIFY(index.isValid());
    QVariant value = index.data(role);
    QCOMPARE(value, expected);
}

void FavoritePlacesModelTest::init()
{
    QString dir = KGlobal::dirs()->localxdgdatadir();
    QFile::remove(dir + "/user-places.xbel");
    QFile::remove(KStandardDirs::locateLocal("data", "kfileplaces/bookmarks.xml"));
}

void FavoritePlacesModelTest::testFavoriteId()
{
    FavoritePlacesModel model;
    model.addPlace("Foo", KUrl("/foo"));
    checkRole(&model, model.rowCount() - 1, FavoritePlacesModel::FavoriteIdRole, "place:file:///foo");

    model.addPlace("Foo", KUrl("sftp://user@example.com/dir/"));
    checkRole(&model, model.rowCount() - 1, FavoritePlacesModel::FavoriteIdRole, "place:sftp://user@example.com/dir/");
}

static bool checkOrder(FavoritePlacesModel *model, const QStringList &list)
{
    int startRow = model->count() - list.count();
    for(int idx = 0; idx < list.count(); ++idx) {
        QModelIndex index = model->index(startRow + idx, 0);
        QString actual = index.data(Qt::DisplayRole).toString();
        QString expected = list[idx];
        if (actual != expected) {
            kWarning() << "--- End of model differs from list ---";
            for(int idx = 0; idx < list.count(); ++idx) {
                QModelIndex index = model->index(startRow + idx, 0);
                QString actual = index.data(Qt::DisplayRole).toString();
                QString expected = list[idx];
                kWarning() << "- actual=" << actual << " expected=" << expected;
            }
            return false;
        }
    }
    return true;
}

void FavoritePlacesModelTest::testMoveRow()
{
    FavoritePlacesModel model;
    int row1 = model.count();
    model.addPlace("Foo", KUrl("/foo"));
    int row2 = model.count();
    model.addPlace("Bar", KUrl("/bar"));
    model.addPlace("Baz", KUrl("/baz"));

    QVERIFY(checkOrder(&model, QStringList() << "Foo" << "Bar" << "Baz"));

    // Move "Foo" after "Bar"
    model.moveRow(row1, row2);
    QVERIFY(checkOrder(&model, QStringList() << "Bar" << "Foo" << "Baz"));

    // Move "Foo" back before "Bar"
    model.moveRow(row2, row1);
    QVERIFY(checkOrder(&model, QStringList() << "Foo" << "Bar" << "Baz"));
}

#include <favoriteplacesmodeltest.moc>
