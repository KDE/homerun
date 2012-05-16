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

void PageModelTest::testLoad()
{
    QScopedPointer<KTemporaryFile> temp(generateTestFile(
        "[Page 0]\n"
        "name=page0\n"
        "icon=icon0\n"
        "modelName=mod0\n"
        "\n"
        "[Page 1]\n"
        "name=page1\n"
        "modelName=mod1\n"
        "modelArgs=arg0,arg1\n"
        ));

    KSharedConfig::Ptr config = KSharedConfig::openConfig(temp->fileName());
    PageModel model;
    model.setConfig(config);

    QCOMPARE(model.rowCount(), 2);

    #define MY_COMPARE(role, value) QCOMPARE(index.data(role).toString(), QString(value))
    QModelIndex index = model.index(0, 0);
    MY_COMPARE(Qt::DisplayRole, "page0");
    MY_COMPARE(PageModel::IconNameRole, "icon0");
    MY_COMPARE(PageModel::ModelNameRole, "mod0");
    QCOMPARE(index.data(PageModel::ModelArgsRole).toStringList(), QStringList());

    index = model.index(1, 0);
    MY_COMPARE(Qt::DisplayRole, "page1");
    MY_COMPARE(PageModel::IconNameRole, "");
    MY_COMPARE(PageModel::ModelNameRole, "mod1");
    QCOMPARE(index.data(PageModel::ModelArgsRole).toStringList(), QStringList() << "arg0" << "arg1");
    #undef MY_COMPARE
}

#include "pagemodeltest.moc"
