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
#include "tabmodeltest.h"

// Local
#include <tabmodel.h>

// KDE
#include <KTemporaryFile>
#include <qtest_kde.h>

// Qt
#include <QSignalSpy>

QTEST_KDEMAIN(TabModelTest, NoGUI)

Q_DECLARE_METATYPE(QModelIndex)

static KTemporaryFile *generateTestFile(const QString &content)
{
    KTemporaryFile *file = new KTemporaryFile;
    Q_ASSERT(file->open());
    file->write(content.toUtf8());
    file->flush();
    return file;
}

void TabModelTest::initTestCase()
{
    qRegisterMetaType<QModelIndex>("QModelIndex");
}

void TabModelTest::testTabOrder()
{
    QScopedPointer<KTemporaryFile> temp(generateTestFile(
        "[Tab0]\n"
        "name=tab0\n"
        "source0=foo\n"
        "[Tab5]\n"
        "name=tab5\n"
        "source0=foo\n"
        "[Tab4]\n"
        "name=tab4\n"
        "source0=foo\n"
        "[Tab2]\n"
        "name=tab2\n"
        "source0=foo\n"
        "[Tab3]\n"
        "name=tab3\n"
        "source0=foo\n"
        "[Tab6]\n"
        "name=tab6\n"
        "source0=foo\n"
        "[Tab1]\n"
        "name=tab1\n"
        "source0=foo\n"
        ));

    KSharedConfig::Ptr config = KSharedConfig::openConfig(temp->fileName());
    TabModel model;
    model.setConfig(config);

    QCOMPARE(model.rowCount(), 7);

    for (int row = 0; row < 7; ++row) {
        QModelIndex index = model.index(row, 0);
        QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("tab%1").arg(row));
    }
}

void TabModelTest::testLoadKeys_data()
{
    QTest::addColumn<QString>("configText");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("iconName");
    QTest::addColumn<QStringList>("sources");

    QTest::newRow("name+icon")
        <<  "[Tab0]\n"
            "name=tab0\n"
            "icon=icon0\n"
            "source0=foo\n"
        << "tab0"
        << "icon0"
        << (QStringList() << "foo");

    QTest::newRow("name-only")
        <<  "[Tab0]\n"
            "name=tab0\n"
            "source0=foo\n"
        << "tab0"
        << QString()
        << (QStringList() << "foo");

    QTest::newRow("multi-sources")
        <<  "[Tab0]\n"
            "name=tab0\n"
            "source0=foo\n"
            "source1=bar\n"
        << "tab0"
        << QString()
        << (QStringList() << "foo" << "bar");
}

void TabModelTest::testLoadKeys()
{
    QFETCH(QString, configText);
    QFETCH(QString, name);
    QFETCH(QString, iconName);
    QFETCH(QStringList, sources);

    // Create config file
    QScopedPointer<KTemporaryFile> temp(generateTestFile(configText));
    KSharedConfig::Ptr config = KSharedConfig::openConfig(temp->fileName());

    // Load it
    TabModel model;
    model.setConfig(config);

    QCOMPARE(model.rowCount(), 1);

    QModelIndex index = model.index(0, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), name);
    QCOMPARE(index.data(Qt::DecorationRole).toString(), iconName);
    QCOMPARE(index.data(TabModel::SourcesRole).toStringList(), sources);
}

void TabModelTest::testSetText()
{
    QString configText =
        "[Tab0]\n"
        "name=first\n"
        "source=foo\n"
        ""
        "[Tab1]\n"
        "name=second\n"
        "source=bar\n"
        ;
    QScopedPointer<KTemporaryFile> temp(generateTestFile(configText));
    KSharedConfig::Ptr config = KSharedConfig::openConfig(temp->fileName());

    // Load it
    TabModel model;
    model.setConfig(config);

    QCOMPARE(model.rowCount(), 2);

    QSignalSpy spy(&model, SIGNAL(dataChanged(QModelIndex, QModelIndex)));
    model.setDataForRow(1, "display", "last");

    QModelIndex index = model.index(1, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("last"));

    QCOMPARE(spy.count(), 1);
    QVariantList args = spy.takeFirst();
    QModelIndex topLeft = args[0].value<QModelIndex>();
    QModelIndex bottomRight = args[1].value<QModelIndex>();
    QCOMPARE(topLeft.row(), 1);
    QCOMPARE(bottomRight.row(), 1);
    QCOMPARE(topLeft.column(), 0);
    QCOMPARE(bottomRight.column(), 0);
}

#include "tabmodeltest.moc"
