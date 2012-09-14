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
#include <multisources.h>

// Homerun
#include <abstractsource.h>
#include <sourceregistry.h>

// KDE
#include <KDebug>
#include <KLocale>
#include <KPluginFactory>

class MultiSourcesSourceA : public AbstractSource
{
public:
    MultiSourcesSourceA(QObject *parent, const QVariantList &args = QVariantList())
    : AbstractSource(parent, args)
    {}

    QAbstractItemModel *createModel(const SourceArguments &)
    {
        return new MultiSourcesModel("MultiA");
    }
};

class MultiSourcesSourceB : public AbstractSource
{
public:
    MultiSourcesSourceB(QObject *parent, const QVariantList &args = QVariantList())
    : AbstractSource(parent, args)
    {}

    QAbstractItemModel *createModel(const SourceArguments &)
    {
        return new MultiSourcesModel("MultiB");
    }
};

K_PLUGIN_FACTORY(factory,
    registerPlugin<MultiSourcesSourceA>("MultiA");
    registerPlugin<MultiSourcesSourceB>("MultiB");
    )
K_EXPORT_PLUGIN(factory("homerun_source_multisources"))

MultiSourcesModel::MultiSourcesModel(const QString &name)
: m_name(name)
{
    // Declare roles
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "label");
    roles.insert(Qt::DecorationRole, "icon");
    setRoleNames(roles);

    // Create an item
    QStandardItem *item;

    item = new QStandardItem;
    item->setText("Lorem Ipsum");
    item->setData("kde", Qt::DecorationRole);
    appendRow(item);
}

bool MultiSourcesModel::trigger(int row)
{
    QStandardItem *theItem = item(row);
    Q_ASSERT(theItem);
    kWarning() << "row clicked:" << row << "item:" << theItem->text();
    return true;
}

QString MultiSourcesModel::name() const
{
    return m_name;
}

int MultiSourcesModel::count() const
{
    return rowCount(QModelIndex());
}

#include <multisources.moc>
