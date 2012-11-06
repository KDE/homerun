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

} // namespace Homerun

#include <pathmodel.moc>
