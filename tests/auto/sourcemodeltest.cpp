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
#include <sourcemodeltest.h>

// Local
#include <abstractsourceregistry.h>
#include <sourcemodel.h>

// KDE
#include <qtest_kde.h>

// Qt
#include <QStandardItemModel>

QTEST_KDEMAIN(SourceModelTest, NoGUI)

class MockRegistry : public Homerun::AbstractSourceRegistry
{
public:
    QObject *createModelFromConfigGroup(const QString &sourceId, const KConfigGroup &/*configGroup*/, QObject *parent)
    {
        if (sourceId == "broken") {
            return 0;
        }
        return new QStandardItemModel(parent);
    }
};

void SourceModelTest::initTestCase()
{
    qRegisterMetaType<QModelIndex>("QModelIndex");
    m_registry = new MockRegistry();
}

void SourceModelTest::cleanupTestCase()
{
    delete m_registry;
}

void SourceModelTest::testAppendSource()
{
    KConfig config(QString(), KConfig::SimpleConfig);
    KConfigGroup group(&config, "Tab0");
    SourceModel model(m_registry, group, 0);

    QCOMPARE(model.rowCount(), 0);
    model.appendSource("foo");

    QCOMPARE(model.rowCount(), 1);

    QCOMPARE(group.readEntry("sources", QStringList()), QStringList() << "Source0");

    KConfigGroup sourceGroup(&group, "Source0");
    QCOMPARE(sourceGroup.readEntry("sourceId"), QString("foo"));
}

#include <sourcemodeltest.moc>
