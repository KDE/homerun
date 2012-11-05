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

static QStringList getTabList(const KSharedConfig::Ptr config)
{
    KConfigGroup group(config, "General");
    return group.readEntry("tabs", QStringList());
}

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
        "[General]\n"
        "tabs=Tab0,Tab1,Tab2,Tab3,Tab4,Tab5,Tab6\n"
        "[Tab0]\n"
        "name=tab0\n"
        "sources=Source0\n"
        "[Tab0][Source0]\n"
        "sourceId=foo\n"
        "\n"
        "[Tab5]\n"
        "name=tab5\n"
        "sources=Source0\n"
        "[Tab5][Source0]\n"
        "sourceId=foo\n"
        "\n"
        "[Tab4]\n"
        "name=tab4\n"
        "sources=Source0\n"
        "[Tab4][Source0]\n"
        "sourceId=foo\n"
        "\n"
        "[Tab2]\n"
        "name=tab2\n"
        "sources=Source0\n"
        "[Tab2][Source0]\n"
        "sourceId=foo\n"
        "\n"
        "[Tab3]\n"
        "name=tab3\n"
        "sources=Source0\n"
        "[Tab3][Source0]\n"
        "sourceId=foo\n"
        "\n"
        "[Tab6]\n"
        "name=tab6\n"
        "sources=Source0\n"
        "[Tab6][Source0]\n"
        "sourceId=foo\n"
        "\n"
        "[Tab1]\n"
        "name=tab1\n"
        "sources=Source0\n"
        "[Tab1][Source0]\n"
        "sourceId=foo\n"
        "\n"
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

static QVariantList makeSourceList(const QString &name, const QString &tabGroupName, const QString &sourceGroupName)
{
    QVariantList sourceList;
    sourceList.append("config");
    sourceList.append(name);
    sourceList.append(
        QStringList() << tabGroupName << sourceGroupName
        );
    return sourceList;
}

template <class List>
static List createList(const List &list1)
{
    List out;
    out.append(QVariant(list1));
    return out;
}
template <class List>
static List createList(const List &list1, const List &list2)
{
    List out;
    out.append(QVariant(list1));
    out.append(QVariant(list2));
    return out;
}
template <class List>
static List createList(const List &list1, const List &list2, const List &list3)
{
    List out;
    out.append(QVariant(list1));
    out.append(QVariant(list2));
    out.append(QVariant(list3));
    return out;
}

void TabModelTest::testLoadKeys_data()
{
    QTest::addColumn<QString>("configText");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("iconName");
    QTest::addColumn<QVariantList>("sources");

    QTest::newRow("name+icon")
        <<  "[General]\n"
            "tabs=Tab0\n"
            "[Tab0]\n"
            "name=tab0\n"
            "icon=icon0\n"
            "sources=Source0\n"
            "[Tab0][Source0]\n"
            "sourceId=foo\n"
        << "tab0"
        << "icon0"
        << createList(makeSourceList("foo", "Tab0", "Source0"));

    QTest::newRow("name-only")
        <<  "[General]\n"
            "tabs=Tab0\n"
            "[Tab0]\n"
            "name=tab0\n"
            "sources=Source0\n"
            "[Tab0][Source0]\n"
            "sourceId=foo\n"
        << "tab0"
        << QString()
        << createList(makeSourceList("foo", "Tab0", "Source0"));

    QTest::newRow("unnamed")
        <<  "[General]\n"
            "tabs=Tab0\n"
            "[Tab0]\n"
            "sources=Source0\n"
            "[Tab0][Source0]\n"
            "sourceId=foo\n"
        << ""
        << QString()
        << createList(makeSourceList("foo", "Tab0", "Source0"));

    QTest::newRow("multi-sources")
        <<  "[General]\n"
            "tabs=Tab0\n"
            "[Tab0]\n"
            "name=tab0\n"
            "sources=Source0,Source1\n"
            "[Tab0][Source0]\n"
            "sourceId=foo\n"
            "[Tab0][Source1]\n"
            "sourceId=bar\n"
        << "tab0"
        << QString()
        << createList(makeSourceList("foo", "Tab0", "Source0"), makeSourceList("bar", "Tab0", "Source1"));

    QTest::newRow("no-sources")
        <<  "[General]\n"
            "tabs=Tab0\n"
            "[Tab0]\n"
            "name=tab0\n"
        << "tab0"
        << QString()
        << QVariantList();
}

void TabModelTest::testLoadKeys()
{
    QFETCH(QString, configText);
    QFETCH(QString, name);
    QFETCH(QString, iconName);
    QFETCH(QVariantList, sources);

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
    QCOMPARE(index.data(TabModel::SourcesRole).value<QVariantList>(), sources);
}

void TabModelTest::testSetDataForRow()
{
    QString configText =
        "[General]\n"
        "tabs=Tab0,Tab1\n"
        ""
        "[Tab0]\n"
        "name=first\n"
        "sources=Source0\n"
        "[Tab0][Source0]\n"
        "sourceId=foo\n"
        ""
        "[Tab1]\n"
        "name=second\n"
        "sources=Source0\n"
        "[Tab1][Source0]\n"
        "sourceId=bar\n"
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
        "[General]\n"
        "tabs=Tab0\n"
        ""
        "[Tab0]\n"
        "name=first\n"
        "sources=Source0\n"
        ""
        "[Tab0][Source0]\n"
        "sourceId=foo\n"
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
    QCOMPARE(getTabList(config), QStringList() << "Tab0" << "Tab1");

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
        "[General]\n"
        "tabs=Tab0,Tab1\n"
        "\n"
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
    QCOMPARE(getTabList(config), QStringList() << "Tab1");

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

static QMap<QString, QString> getEntries(KSharedConfig::Ptr config, const QString &tabName)
{
    return config->group(tabName).entryMap();
}

static QStringList getSources(const QModelIndex &index)
{
    return index.data(TabModel::SourcesRole).value<QStringList>();
}

void TabModelTest::testMoveRow()
{
    QFETCH(int, from);
    QFETCH(int, to);

    QString configText =
        "[General]\n"
        "tabs=Tab0,Tab1,Tab2\n"
        "\n"
        "[Tab0]\n"
        "name=zero\n"
        "source0=tab0-0\n"
        "source1=tab0-1\n"
        "source2=tab0-2\n"
        "\n"
        "[Tab1]\n"
        "name=one\n"
        "icon=iconOne\n"
        "source0=tab1-0\n"
        "source1=tab1-1\n"
        "\n"
        "[Tab2]\n"
        "name=two\n"
        "icon=iconTwo\n"
        "source0=tab2-0\n"
        ;
    QScopedPointer<KTemporaryFile> temp(generateTestFile(configText));
    KSharedConfig::Ptr config = KSharedConfig::openConfig(temp->fileName());

    // Load it
    TabModel model;
    model.setConfig(config);
    QCOMPARE(model.rowCount(), 3);

    QStringList beforeTabs = getTabList(config);
    QMap<QString, QString> beforeFrom = getEntries(config, beforeTabs.at(from));
    QMap<QString, QString> beforeTo = getEntries(config, beforeTabs.at(to));
    QStringList beforeFromSources = getSources(model.index(from, 0));
    QVERIFY(!beforeFromSources.isEmpty());
    QStringList beforeToSources = getSources(model.index(to, 0));
    QVERIFY(!beforeToSources.isEmpty());

    QSignalSpy aboutSpy(&model, SIGNAL(rowsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)));
    QSignalSpy spy(&model, SIGNAL(rowsMoved(QModelIndex, int, int, QModelIndex, int)));
    model.moveRow(from, to);

    // Check model
    QModelIndex index;
    index = model.index(to, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), beforeFrom.value("name"));
    QCOMPARE(getSources(index), beforeFromSources);
    index = model.index(from, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), beforeTo.value("name"));
    QCOMPARE(getSources(index), beforeToSources);

    // Check config
    QStringList afterTabs = getTabList(config);
    QStringList expectedTabs = beforeTabs;
    expectedTabs.swap(from, to);
    QCOMPARE(afterTabs, expectedTabs);

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

    // Reload config
    TabModel model2;
    model2.setConfig(config);
    QMap<QString, QString> afterFrom = getEntries(config, afterTabs.at(from));
    QMap<QString, QString> afterTo = getEntries(config, afterTabs.at(to));
    QCOMPARE(beforeFrom, afterTo);
    QCOMPARE(beforeTo, afterFrom);
    QStringList afterFromSources = getSources(model2.index(from, 0));
    QStringList afterToSources = getSources(model2.index(to, 0));
    QCOMPARE(beforeFromSources, afterToSources);
    QCOMPARE(beforeToSources, afterFromSources);
}

void TabModelTest::testLoadLegacy()
{
    QString configText =
        "[General]\n"
        "\n"
        "[Tab0]\n"
        "name=zero\n"
        "source0=zero0\n"
        "source1=zero1\n"
        "\n"
        "[Tab1]\n"
        "name=one\n"
        "icon=iconOne\n"
        "source0=one\n"
        "\n"
        "[Tab2]\n"
        "deleted=true\n"
        "\n"
        "[Tab3]\n"
        "name=three\n"
        "icon=iconThree\n"
        "source0=three:arg1=value1\n"
        ;

    QScopedPointer<KTemporaryFile> temp(generateTestFile(configText));
    KSharedConfig::Ptr config = KSharedConfig::openConfig(temp->fileName());

    // Load it
    TabModel model;
    model.setConfig(config);
    QCOMPARE(model.rowCount(), 3);

    QStringList tabList = getTabList(config);
    QCOMPARE(tabList, QStringList() << "Tab0" << "Tab1" << "Tab3");


    QVector<QVariantList> expectedSourceLists;

    expectedSourceLists
        << createList(makeSourceList("zero0", "Tab0", "Source0"), makeSourceList("zero1", "Tab0", "Source1"))
        << createList(makeSourceList("one", "Tab1", "Source0"))
        << createList(makeSourceList("three", "Tab3", "Source0"))
        ;
    for (int row = 0; row < model.rowCount(); ++row) {
        QVariantList sourceLists = model.index(row, 0).data(TabModel::SourcesRole).value<QVariantList>();
        QCOMPARE(expectedSourceLists.at(row), sourceLists);
    }
}

#include "tabmodeltest.moc"
