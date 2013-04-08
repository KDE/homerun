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
    QCOMPARE(expected, value);
}

void FavoritePlacesModelTest::init()
{
    QString dir = KGlobal::dirs()->localxdgdatadir();
    QFile file(dir + "/user-places.xbel");
    file.remove();
}

void FavoritePlacesModelTest::testFavoriteId()
{
    FavoritePlacesModel model;
    model.addPlace("Foo", KUrl("/foo"));
    checkRole(&model, model.rowCount() - 1, FavoritePlacesModel::FavoriteIdRole, "place:file:///foo");

    model.addPlace("Foo", KUrl("sftp://user@example.com/dir/"));
    checkRole(&model, model.rowCount() - 1, FavoritePlacesModel::FavoriteIdRole, "place:sftp://user@example.com/dir/");
}

#include <favoriteplacesmodeltest.moc>
