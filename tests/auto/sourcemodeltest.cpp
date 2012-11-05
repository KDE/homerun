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
}

#include <sourcemodeltest.moc>
