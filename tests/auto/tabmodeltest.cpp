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

#include <KTemporaryFile>
#include <qtest_kde.h>

#include <tabmodel.h>

QTEST_KDEMAIN(TabModelTest, NoGUI)

static KTemporaryFile *generateTestFile(const QString &content)
{
    KTemporaryFile *file = new KTemporaryFile;
    Q_ASSERT(file->open());
    file->write(content.toUtf8());
    file->flush();
    return file;
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
    QTest::addColumn<QStringList>("searchSources");

    QTest::newRow("name+icon")
        <<  "[Tab0]\n"
            "name=tab0\n"
            "icon=icon0\n"
            "source0=foo\n"
        << "tab0"
        << "icon0"
        << (QStringList() << "foo")
        << QStringList();

    QTest::newRow("name-only")
        <<  "[Tab0]\n"
            "name=tab0\n"
            "source0=foo\n"
        << "tab0"
        << QString()
        << (QStringList() << "foo")
        << QStringList();

    QTest::newRow("multi-sources")
        <<  "[Tab0]\n"
            "name=tab0\n"
            "source0=foo\n"
            "source1=bar\n"
            "searchSource0=baz\n"
            "searchSource1=boom\n"
        << "tab0"
        << QString()
        << (QStringList() << "foo" << "bar")
        << (QStringList() << "baz" << "boom");
}

void TabModelTest::testLoadKeys()
{
    QFETCH(QString, configText);
    QFETCH(QString, name);
    QFETCH(QString, iconName);
    QFETCH(QStringList, sources);
    QFETCH(QStringList, searchSources);

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
    QCOMPARE(index.data(TabModel::SearchSourcesRole).toStringList(), searchSources);
}

#include "tabmodeltest.moc"
