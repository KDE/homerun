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
#ifndef TABMODELTEST_H
#define TABMODELTEST_H

#include <QObject>

class MockRegistry;

class TabModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testTabOrder();
    void testLoadKeys();
    void testLoadKeys_data();

    void testSetDataForRow();

    void testAppendRow();
    void testRemoveRow();
    void testMoveRow_data();
    void testMoveRow();
    void testAppendRowToEmptyModel();

    void testResetConfig();

private:
    MockRegistry *m_registry;
};

#endif /* TABMODELTEST_H */
