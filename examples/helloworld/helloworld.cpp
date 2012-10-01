/*
Copyright 2012 Aurélien Gâteau <agateau@kde.org>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
// Self
#include <helloworld.h>

// Homerun
#include <abstractsource.h>

// KDE
#include <KDebug>
#include <KLocale>
#include <KPluginFactory>

typedef Homerun::SimpleSource<HelloWorldModel> HelloWorldSource;
// First argument is the library name without the "homerun_source_" prefix
// Second argument is the class name of the source
HOMERUN_EXPORT_SOURCE(helloworld, HelloWorldSource)

HelloWorldModel::HelloWorldModel()
{
    // Create two items
    QStandardItem *item;

    item = new QStandardItem;
    item->setText(i18n("Hello"));
    item->setData("face-smile", Qt::DecorationRole);
    appendRow(item);

    item = new QStandardItem;
    item->setText(i18n("World"));
    item->setData("network-defaultroute", Qt::DecorationRole);
    appendRow(item);
}

bool HelloWorldModel::trigger(int row)
{
    QStandardItem *theItem = item(row);
    Q_ASSERT(theItem);
    kWarning() << "row clicked:" << row << "item:" << theItem->text();
    return true;
}

QString HelloWorldModel::name() const
{
    return i18n("Hello World");
}

int HelloWorldModel::count() const
{
    return rowCount(QModelIndex());
}

#include <helloworld.moc>
