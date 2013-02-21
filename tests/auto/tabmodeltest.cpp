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
#include "tabmodeltest.h"

// Local
#include <abstractsourceregistry.h>
#include <customtypes.h>
#include <sourcemodel.h>
#include <tabmodel.h>

// KDE
#include <KConfigGroup>
#include <KDebug>
#include <KTemporaryFile>
#include <KTempDir>
#include <KStandardDirs>
#include <qtest_kde.h>

// Qt
#include <QSignalSpy>
#include <QStandardItemModel>

QTEST_KDEMAIN(TabModelTest, NoGUI)

Q_DECLARE_METATYPE(QModelIndex)

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

static QStringList getSourceGroups(const QModelIndex &index)
{
    QAbstractItemModel *tabSourceModel = qobject_cast<QAbstractItemModel *>(
        index.data(TabModel::SourceModelRole).value<QObject *>()
        );
    if (!tabSourceModel) {
        kError() << "No tab model";
        return QStringList();
    }
    QStringList lst;
    for (int row = 0; row < tabSourceModel->rowCount(); ++row) {
        QModelIndex index = tabSourceModel->index(row, 0);
        KConfigGroup *group = index.data(SourceModel::ConfigGroupRole).value<KConfigGroup *>();
        Q_ASSERT(!group->name().isEmpty());
        lst << group->parent().name() + "/" + group->name();
    }
    return lst;
}

static QStringList getTabList(const KSharedConfig::Ptr config)
{
    KConfigGroup group(config, "General");
    return group.readEntry("tabs", QStringList());
}

static KTemporaryFile *generateTestFile(const QString &content)
{
    KTemporaryFile *file = new KTemporaryFile;
    bool ok = file->open();
    Q_ASSERT(ok);
    file->write(content.toUtf8());
    file->flush();
    return file;
}

void writeFile(const QString &fileName, const QString &content)
{
    QFile file(fileName);
    bool ok = file.open(QIODevice::WriteOnly);
    Q_ASSERT(ok);
    file.write(content.toUtf8());
}

static void printFile(const QString &fileName)
{
    QFile file(fileName);
    bool ok = file.open(QIODevice::ReadOnly);
    Q_ASSERT(ok);
    kWarning() << file.readAll();
}

void TabModelTest::initTestCase()
{
    qRegisterMetaType<QModelIndex>("QModelIndex");
    m_registry = new MockRegistry();
}

void TabModelTest::cleanupTestCase()
{
    delete m_registry;
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
    model.setSourceRegistry(m_registry);
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
        << (QStringList() << "Tab0/Source0");

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
        << (QStringList() << "Tab0/Source0");

    QTest::newRow("unnamed")
        <<  "[General]\n"
            "tabs=Tab0\n"
            "[Tab0]\n"
            "sources=Source0\n"
            "[Tab0][Source0]\n"
            "sourceId=foo\n"
        << ""
        << QString()
        << (QStringList() << "Tab0/Source0");

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
        << (QStringList() << "Tab0/Source0" << "Tab0/Source1");

    QTest::newRow("no-sources")
        <<  "[General]\n"
            "tabs=Tab0\n"
            "[Tab0]\n"
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
    model.setSourceRegistry(m_registry);
    model.setConfig(config);

    QCOMPARE(model.rowCount(), 1);

    QModelIndex index = model.index(0, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), name);
    QCOMPARE(index.data(Qt::DecorationRole).toString(), iconName);
    QStringList lst = getSourceGroups(index);
    QCOMPARE(sources, lst);
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
    model.setSourceRegistry(m_registry);
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
    model.setSourceRegistry(m_registry);
    model.setConfig(config);
    QCOMPARE(model.rowCount(), 1);

    QSignalSpy aboutInsertedSpy(&model, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)));
    QSignalSpy insertedSpy(&model, SIGNAL(rowsInserted(QModelIndex, int, int)));

    // Append row
    model.appendRow();

    // Check model
    QCOMPARE(model.rowCount(), 2);
    QVariant sourceModelVariant = model.index(1, 0).data(TabModel::SourceModelRole);
    QVERIFY(sourceModelVariant.value<QObject*>());

    // Check config
    QVERIFY(config->hasGroup("Tab1"));
    QVERIFY(config->group("Tab1").hasKey("name"));
    QString name = config->group("Tab1").readEntry("name");
    QCOMPARE(name, QString());
    QCOMPARE(getTabList(config), QStringList() << "Tab0" << "Tab1");

    // Check signals
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
        "sources=foo\n"
        "\n"
        "[Tab1]\n"
        "name=second\n"
        "sources=bar\n"
        ;
    QScopedPointer<KTemporaryFile> temp(generateTestFile(configText));
    KSharedConfig::Ptr config = KSharedConfig::openConfig(temp->fileName());

    // Load it
    TabModel model;
    model.setSourceRegistry(m_registry);
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
    model2.setSourceRegistry(m_registry);
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
        "sources=tab0-0,tab0-1,tab0-2\n"
        "\n"
        "[Tab1]\n"
        "name=one\n"
        "icon=iconOne\n"
        "sources=tab1-0,tab1-1\n"
        "\n"
        "[Tab2]\n"
        "name=two\n"
        "icon=iconTwo\n"
        "sources=tab2-0\n"
        ;
    QScopedPointer<KTemporaryFile> temp(generateTestFile(configText));
    KSharedConfig::Ptr config = KSharedConfig::openConfig(temp->fileName());

    // Load it
    TabModel model;
    model.setSourceRegistry(m_registry);
    model.setConfig(config);
    QCOMPARE(model.rowCount(), 3);

    QStringList beforeTabs = getTabList(config);
    QMap<QString, QString> beforeFrom = getEntries(config, beforeTabs.at(from));
    QMap<QString, QString> beforeTo = getEntries(config, beforeTabs.at(to));
    QStringList beforeFromSources = getSourceGroups(model.index(from, 0));
    QVERIFY(!beforeFromSources.isEmpty());
    QStringList beforeToSources = getSourceGroups(model.index(to, 0));
    QVERIFY(!beforeToSources.isEmpty());

    QSignalSpy aboutSpy(&model, SIGNAL(rowsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)));
    QSignalSpy spy(&model, SIGNAL(rowsMoved(QModelIndex, int, int, QModelIndex, int)));
    model.moveRow(from, to);

    // Check model
    QModelIndex index;
    index = model.index(to, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), beforeFrom.value("name"));
    QCOMPARE(getSourceGroups(index), beforeFromSources);
    index = model.index(from, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), beforeTo.value("name"));
    QCOMPARE(getSourceGroups(index), beforeToSources);

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
    model2.setSourceRegistry(m_registry);
    model2.setConfig(config);
    QMap<QString, QString> afterFrom = getEntries(config, afterTabs.at(from));
    QMap<QString, QString> afterTo = getEntries(config, afterTabs.at(to));
    QCOMPARE(beforeFrom, afterTo);
    QCOMPARE(beforeTo, afterFrom);
    QStringList afterFromSources = getSourceGroups(model2.index(from, 0));
    QStringList afterToSources = getSourceGroups(model2.index(to, 0));
    QCOMPARE(beforeFromSources, afterToSources);
    QCOMPARE(beforeToSources, afterFromSources);
}

void TabModelTest::testAppendRowToEmptyModel()
{
    QScopedPointer<KTemporaryFile> temp(generateTestFile(QString()));
    KSharedConfig::Ptr config = KSharedConfig::openConfig(temp->fileName());

    // Load it
    TabModel model;
    model.setSourceRegistry(m_registry);
    model.setConfig(config);
    QCOMPARE(model.rowCount(), 0);

    // Append row
    model.appendRow();

    // Check model
    QCOMPARE(model.rowCount(), 1);

    QStringList tabList = getTabList(config);
    QCOMPARE(tabList, QStringList() << "Tab0");
}

void TabModelTest::testResetConfig()
{
    KSharedConfig::Ptr config;
    // setup system and user configs
    KTempDir systemDir;
    QString configrc = "testhomerunrc";

    writeFile(systemDir.name() + configrc,
        "[General]\n"
        "tabs=Tab0,Tab1,Tab2\n"
        "\n"
        "[Tab0]\n"
        "name=zero\n"
        "icon=iconZero\n"
        "sources=tab0-0,tab0-1,tab0-2\n"
        "\n"
        "[Tab1]\n"
        "name=one\n"
        "icon=iconOne\n"
        "sources=tab1-0,tab1-1\n"
        "\n"
        "[Tab2]\n"
        "name=two\n"
        "icon=iconTwo\n"
        "sources=Source0\n"
        "\n"
        "[Tab2][Source0]\n"
        "sourceId=ShipList\n"
        "includeMotherShip=true\n"
    );

    writeFile(KStandardDirs::locateLocal("config", configrc),
        "[General]\n"
        "tabs=Tab0,Tab2,Tab3,Tab5\n"
        "\n"
        "[Tab0]\n"
        "name=Foo\n"
        "sources=tab0-0,tab0-2\n"
        "\n"
        "[Tab1]\n"
        "name[$d]\n"
        "sources[$d]\n"
        "\n"
        "[Tab2]\n"
        "name=Bar\n"
        "icon=iconBar\n"
        "sources=Source0,Source1\n"
        "\n"
        "[Tab2][Source0]\n"
        "sourceId=CarList\n"
        "\n"
        "[Tab3]\n"
        "name=Baz\n"
        "icon=iconBaz\n"
        "sources=tab3-0\n"
        "\n"
        "[Tab4]\n"
        "name[$d]\n"
        "sources[$d]\n"
        "\n"
        "[Tab5]\n"
        "name=Boom\n"
        "icon=iconBoom\n"
        "sources=Source1\n"
        "\n"
        "[Tab5][Source1]\n"
        "sourceId=CylonStatus\n"
        "detailLevel=4\n"
    );
    KGlobal::dirs()->addResourceDir("config", systemDir.name());
    config = KSharedConfig::openConfig(configrc);

    // Create model from config
    TabModel model;
    model.setSourceRegistry(m_registry);
    model.setConfig(config);

    // Reset config, we should be back to the system config
    model.resetConfig();

    if (0) {
        printFile(KStandardDirs::locateLocal("config", configrc));
    }

    // Check model
    QCOMPARE(model.rowCount(), 3);
    QModelIndex index;

    index = model.index(0, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("zero"));
    QCOMPARE(index.data(Qt::DecorationRole).toString(), QString("iconZero"));
    QCOMPARE(getSourceGroups(index), QStringList() << "Tab0/tab0-0" << "Tab0/tab0-1" << "Tab0/tab0-2");

    index = model.index(1, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("one"));
    QCOMPARE(index.data(Qt::DecorationRole).toString(), QString("iconOne"));
    QCOMPARE(getSourceGroups(index), QStringList() << "Tab1/tab1-0" << "Tab1/tab1-1");

    index = model.index(2, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("two"));
    QCOMPARE(index.data(Qt::DecorationRole).toString(), QString("iconTwo"));
    QCOMPARE(getSourceGroups(index), QStringList() << "Tab2/Source0");
    QAbstractItemModel *sourceModel = qobject_cast<QAbstractItemModel *>(index.data(TabModel::SourceModelRole).value<QObject *>());
    Q_ASSERT(sourceModel);
    QCOMPARE(sourceModel->rowCount(QModelIndex()), 1);
    QCOMPARE(sourceModel->index(0, 0).data(SourceModel::SourceIdRole).toString(), QString("ShipList"));

    // Check config file
    QCOMPARE(getTabList(config), QStringList() << "Tab0" << "Tab1" << "Tab2");
}

#include "tabmodeltest.moc"
