/*
Copyright 2012 Aurélien Gâteau <agateau@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
// Self
#include <pathmodel.h>

// Local

// KDE

// Qt

namespace Homerun {

enum {
    SourceIdRole = Qt::UserRole + 1,
    SourceArgumentsRole
};

struct PathModelPrivate
{
};

PathModel::PathModel(QObject *parent)
: QStandardItemModel(parent)
, d(new PathModelPrivate)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(SourceIdRole, "sourceId");
    roles.insert(SourceArgumentsRole, "sourceArguments");
    setRoleNames(roles);
    connect(this, SIGNAL(modelReset()), SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsInserted(QModelIndex, int, int)), SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex, int, int)), SIGNAL(countChanged()));
}

PathModel::~PathModel()
{
    delete d;
}

void PathModel::addPath(const QString &label, const QString &sourceId, const QVariantMap &sourceArguments)
{
    QStandardItem *item = new QStandardItem(label);
    item->setData(sourceId, SourceIdRole);
    item->setData(sourceArguments, SourceArgumentsRole);
    appendRow(item);
}

int PathModel::count() const
{
    return rowCount();
}

QVariantHash PathModel::get(int row) const
{
    QStandardItem *itm = item(row);
    Q_ASSERT(itm);
    QVariantHash hash;
    hash["display"] = itm->text();
    hash["sourceId"] = itm->data(SourceIdRole).toString();
    hash["sourceArguments"] = itm->data(SourceArgumentsRole).toMap();
    return hash;
}

} // namespace Homerun

#include <pathmodel.moc>
