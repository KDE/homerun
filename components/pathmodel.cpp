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

PathModel::PathModel(QObject *parent)
: QStandardItemModel(parent)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "label");
    roles.insert(SourceRole, "source");
    setRoleNames(roles);
}

PathModel::~PathModel()
{
}

void PathModel::addPath(const QString &label, const QString &source)
{
    QStandardItem *item = new QStandardItem(label);
    item->setData(source, SourceRole);
    appendRow(item);
}

#include <pathmodel.moc>
