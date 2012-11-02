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

void FavoritePlacesModelTest::initTestCase()
{
    QString dir = KGlobal::dirs()->localxdgdatadir();
    QFile file(dir + "/user-places.xbel");
    file.remove();
}

void FavoritePlacesModelTest::testFavoriteId()
{
    FavoritePlacesModel model;
    {
        model.addPlace("Foo", KUrl("/foo"));
        QModelIndex index = model.index(model.rowCount() - 1, 0);
        QString favoriteId = index.data(FavoritePlacesModel::FavoriteIdRole).toString();
        QCOMPARE(favoriteId, QString("place:file:///foo"));
    }
    {
        model.addPlace("Foo", KUrl("sftp://user@example.com/dir/"));
        QModelIndex index = model.index(model.rowCount() - 1, 0);
        QString favoriteId = index.data(FavoritePlacesModel::FavoriteIdRole).toString();
        QCOMPARE(favoriteId, QString("place:sftp://user@example.com/dir/"));
    }
}

#include <favoriteplacesmodeltest.moc>
