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
#include "pagemodeltest.h"

#include <KTemporaryFile>
#include <qtest_kde.h>

#include <pagemodel.h>

QTEST_KDEMAIN(PageModelTest, NoGUI)

static KTemporaryFile *generateTestFile(const QString &content)
{
    KTemporaryFile *file = new KTemporaryFile;
    Q_ASSERT(file->open());
    file->write(content.toUtf8());
    file->flush();
    return file;
}

void PageModelTest::testPageOrder()
{
    QScopedPointer<KTemporaryFile> temp(generateTestFile(
        "[Page0]\n"
        "name=page0\n"
        "source0=foo\n"
        "[Page5]\n"
        "name=page5\n"
        "source0=foo\n"
        "[Page4]\n"
        "name=page4\n"
        "source0=foo\n"
        "[Page2]\n"
        "name=page2\n"
        "source0=foo\n"
        "[Page3]\n"
        "name=page3\n"
        "source0=foo\n"
        "[Page6]\n"
        "name=page6\n"
        "source0=foo\n"
        "[Page1]\n"
        "name=page1\n"
        "source0=foo\n"
        ));

    KSharedConfig::Ptr config = KSharedConfig::openConfig(temp->fileName());
    PageModel model;
    model.setConfig(config);

    QCOMPARE(model.rowCount(), 7);

    for (int row = 0; row < 7; ++row) {
        QModelIndex index = model.index(row, 0);
        QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("page%1").arg(row));
    }
}

void PageModelTest::testLoadKeys_data()
{
    QTest::addColumn<QString>("configText");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("iconName");
    QTest::addColumn<QStringList>("sources");
    QTest::addColumn<QStringList>("searchSources");

    QTest::newRow("name+icon")
        <<  "[Page0]\n"
            "name=page0\n"
            "icon=icon0\n"
            "source0=foo\n"
        << "page0"
        << "icon0"
        << (QStringList() << "foo")
        << QStringList();

    QTest::newRow("name-only")
        <<  "[Page0]\n"
            "name=page0\n"
            "source0=foo\n"
        << "page0"
        << QString()
        << (QStringList() << "foo")
        << QStringList();

    QTest::newRow("multi-sources")
        <<  "[Page0]\n"
            "name=page0\n"
            "source0=foo\n"
            "source1=bar\n"
            "searchSource0=baz\n"
            "searchSource1=boom\n"
        << "page0"
        << QString()
        << (QStringList() << "foo" << "bar")
        << (QStringList() << "baz" << "boom");
}

void PageModelTest::testLoadKeys()
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
    PageModel model;
    model.setConfig(config);

    QCOMPARE(model.rowCount(), 1);

    QModelIndex index = model.index(0, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), name);
    QCOMPARE(index.data(PageModel::IconNameRole).toString(), iconName);
    QCOMPARE(index.data(PageModel::SourcesRole).toStringList(), sources);
    QCOMPARE(index.data(PageModel::SearchSourcesRole).toStringList(), searchSources);
}

#include "pagemodeltest.moc"
