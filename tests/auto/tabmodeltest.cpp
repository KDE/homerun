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
#include <KConfigGroup>
#include <KDebug>
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

    QTest::newRow("unnamed")
        <<  "[Tab0]\n"
            "source0=foo\n"
        << ""
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

    QTest::newRow("no-sources")
        <<  "[Tab0]\n"
            "name=tab0\n"
        << "tab0"
        << QString()
        << QStringList();
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

void TabModelTest::testSetDataForRow()
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
    model.setDataForRow(1, "decoration", "document-new");

    QModelIndex index = model.index(1, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("last"));
    QCOMPARE(index.data(Qt::DecorationRole).toString(), QString("document-new"));

    QCOMPARE(spy.count(), 2);
    QVariantList args = spy.takeFirst();
    QModelIndex topLeft = args[0].value<QModelIndex>();
    QModelIndex bottomRight = args[1].value<QModelIndex>();
    QCOMPARE(topLeft.row(), 1);
    QCOMPARE(bottomRight.row(), 1);
    QCOMPARE(topLeft.column(), 0);
    QCOMPARE(bottomRight.column(), 0);
}

void TabModelTest::testAppendRow()
{
    QString configText =
        "[Tab0]\n"
        "name=first\n"
        "source=foo\n"
        ;
    QScopedPointer<KTemporaryFile> temp(generateTestFile(configText));
    KSharedConfig::Ptr config = KSharedConfig::openConfig(temp->fileName());

    // Load it
    TabModel model;
    model.setConfig(config);
    QCOMPARE(model.rowCount(), 1);

    QSignalSpy aboutInsertedSpy(&model, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)));
    QSignalSpy insertedSpy(&model, SIGNAL(rowsInserted(QModelIndex, int, int)));
    model.appendRow();
    QCOMPARE(model.rowCount(), 2);

    QVERIFY(config->hasGroup("Tab1"));
    QVERIFY(config->group("Tab1").hasKey("name"));
    QString name = config->group("Tab1").readEntry("name");
    QCOMPARE(name, QString());

    QCOMPARE(aboutInsertedSpy.count(), 1);
    QVariantList args = aboutInsertedSpy.takeFirst();
    QVERIFY(!args[0].value<QModelIndex>().isValid());
    QCOMPARE(args[1].toInt(), 1);
    QCOMPARE(args[2].toInt(), 1);

    QCOMPARE(insertedSpy.count(), 1);
    args = insertedSpy.takeFirst();
    QVERIFY(!args[0].value<QModelIndex>().isValid());
    QCOMPARE(args[1].toInt(), 1);
    QCOMPARE(args[2].toInt(), 1);
}

void TabModelTest::testRemoveRow()
{
    QString configText =
        "[Tab0]\n"
        "name=first\n"
        "source=foo\n"
        "\n"
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

    // Remove row
    QSignalSpy aboutRemovedSpy(&model, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)));
    QSignalSpy removedSpy(&model, SIGNAL(rowsRemoved(QModelIndex, int, int)));
    model.removeRow(0);
    QCOMPARE(model.rowCount(), 1);

    // Check config file
    QVERIFY(config->group("Tab0").readEntry("deleted", false));
    QVERIFY(!config->group("Tab1").readEntry("deleted", false));

    // Check signals
    QCOMPARE(aboutRemovedSpy.count(), 1);
    QVariantList args = aboutRemovedSpy.takeFirst();
    QVERIFY(!args[0].value<QModelIndex>().isValid());
    QCOMPARE(args[1].toInt(), 0);
    QCOMPARE(args[2].toInt(), 0);

    QCOMPARE(removedSpy.count(), 1);
    args = removedSpy.takeFirst();
    QVERIFY(!args[0].value<QModelIndex>().isValid());
    QCOMPARE(args[1].toInt(), 0);
    QCOMPARE(args[2].toInt(), 0);

    // Load again
    TabModel model2;
    model2.setConfig(config);
    QCOMPARE(model2.rowCount(), 1);
}

void TabModelTest::testMoveRow_data()
{
    QTest::addColumn<int>("from");
    QTest::addColumn<int>("to");

    QTest::newRow("2to1") << 2 << 1;
    QTest::newRow("1to2") << 1 << 2;
}

static QMap<QString, QString> getEntries(KSharedConfig::Ptr config, int row)
{
    return config->group("Tab" + QString::number(row)).entryMap();
}

void TabModelTest::testMoveRow()
{
    QFETCH(int, from);
    QFETCH(int, to);

    QString configText =
        "[Tab0]\n"
        "name=zero\n"
        "source0=zero\n"
        "\n"
        "[Tab1]\n"
        "name=one\n"
        "icon=iconOne\n"
        "source0=one\n"
        "\n"
        "[Tab2]\n"
        "name=two\n"
        "icon=iconTwo\n"
        "source0=two\n"
        ;
    QScopedPointer<KTemporaryFile> temp(generateTestFile(configText));
    KSharedConfig::Ptr config = KSharedConfig::openConfig(temp->fileName());

    // Load it
    TabModel model;
    model.setConfig(config);
    QCOMPARE(model.rowCount(), 3);

    QMap<QString, QString> beforeFrom = getEntries(config, from);
    QMap<QString, QString> beforeTo = getEntries(config, to);

    QSignalSpy aboutSpy(&model, SIGNAL(rowsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)));
    QSignalSpy spy(&model, SIGNAL(rowsMoved(QModelIndex, int, int, QModelIndex, int)));
    model.moveRow(from, to);

    // Check model
    QModelIndex index;
    index = model.index(to, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), beforeFrom.value("name"));
    index = model.index(from, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), beforeTo.value("name"));

    // Check config
    QMap<QString, QString> afterFrom = getEntries(config, from);
    QMap<QString, QString> afterTo = getEntries(config, to);
    QCOMPARE(afterFrom["deleted"], QString("false"));
    afterFrom.remove("deleted");
    QCOMPARE(afterTo["deleted"], QString("false"));
    afterTo.remove("deleted");
    QCOMPARE(beforeFrom, afterTo);
    QCOMPARE(beforeTo, afterFrom);

    // Check signals
    int modelTo = to + (to > from ? 1 : 0);
    QVariantList args;
    QCOMPARE(aboutSpy.count(), 1);
    args = aboutSpy.takeFirst();
    QVERIFY(!args[0].value<QModelIndex>().isValid());
    QCOMPARE(args[1].toInt(), from);
    QCOMPARE(args[2].toInt(), from);
    QVERIFY(!args[3].value<QModelIndex>().isValid());
    QCOMPARE(args[4].toInt(), modelTo);

    QCOMPARE(spy.count(), 1);
    args = spy.takeFirst();
    QVERIFY(!args[0].value<QModelIndex>().isValid());
    QCOMPARE(args[1].toInt(), from);
    QCOMPARE(args[2].toInt(), from);
    QVERIFY(!args[3].value<QModelIndex>().isValid());
    QCOMPARE(args[4].toInt(), modelTo);
}

#include "tabmodeltest.moc"
