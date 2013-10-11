/*
 *   Copyright 2013 Eike Hein <hein@kde.org>
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "sourcelistmodel.h"

SourceListModel::SourceListModel(QObject* parent): QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(ModelRole, "model");
    roles.insert(CountRole, "count");

    setRoleNames(roles);
}

SourceListModel::~SourceListModel()
{
}

int SourceListModel::count() const
{
    return rowCount();
}

int SourceListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_sourceList.count();
}

QVariant SourceListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() > m_sourceList.count() - 1) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return m_sourceList.at(index.row()).first;
    } else if (role == ModelRole) {
        return QVariant::fromValue(m_sourceList.at(index.row()).second);
    } else if (role == CountRole) {
        return m_sourceList.at(index.row()).second->property("count");
    }

    return QVariant();
}

void SourceListModel::appendSource(const QString& name, QObject* model)
{
    insertSource(m_sourceList.count(), name, model);
}

void SourceListModel::insertSource(int index, const QString& name, QObject* model)
{
    connect(model, SIGNAL(destroyed()), this, SLOT(handleModelDestruction()));
    connect(model, SIGNAL(countChanged()), this, SLOT(handleCountChange()));

    beginInsertRows(QModelIndex(), index, index);

    QPair<QString, QObject*> source;
    source.first = name;
    source.second = model;

    m_sourceList.insert(index, source);

    endInsertRows();

    emit countChanged();
}

QObject* SourceListModel::modelForRow(int row) const
{
    if (row < m_sourceList.count()) {
        return m_sourceList.at(row).second;
    }

    return 0;
}

void SourceListModel::handleModelDestruction()
{
    QObject* model = QObject::sender();

    int index = -1;

    for (int i = 0; i < m_sourceList.count(); ++i) {
        if (m_sourceList.at(i).second == model) {
            index = i;
            break;
        }
    }

    beginRemoveRows(QModelIndex(), index, index);

    m_sourceList.removeAt(index);

    endRemoveRows();

    emit countChanged();
}

void SourceListModel::handleCountChange()
{
    QObject* model = QObject::sender();

    int index = -1;

    for (int i = 0; i < m_sourceList.count(); ++i) {
        if (m_sourceList.at(i).second == model) {
            index = i;
            break;
        }
    }

    QModelIndex modelIndex = createIndex(index, 0);

    emit dataChanged(modelIndex, modelIndex);
}

SourceListFilterModel::SourceListFilterModel(QObject* parent): QSortFilterProxyModel(parent),
    m_sourceListModel(new SourceListModel(this))
{
    setSourceModel(m_sourceListModel);
    setDynamicSortFilter(true);
    setRoleNames(m_sourceListModel->roleNames());

    connect(this, SIGNAL(modelReset()), this, SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SIGNAL(countChanged()));
}

SourceListFilterModel::~SourceListFilterModel()
{
}

int SourceListFilterModel::count() const
{
    return rowCount();
}

void SourceListFilterModel::appendSource(const QString& name, QObject* model)
{
    m_sourceListModel->appendSource(name, model);
}

void SourceListFilterModel::insertSource(int index, const QString& name, QObject* model)
{
    m_sourceListModel->insertSource(index, name, model);
}

QObject* SourceListFilterModel::modelForRow(int row) const
{
    if (row > m_sourceListModel->count() - 1) {
        return 0;
    }

    QModelIndex mapped = mapToSource(index(row, 0));

    if (!mapped.isValid()) {
        return 0;
    }

    return m_sourceListModel->modelForRow(mapped.row());
}

bool SourceListFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    Q_UNUSED(source_parent)

    int count = m_sourceListModel->data(m_sourceListModel->index(source_row, 0), Qt::UserRole + 1).toInt();

    return (count > 0);
}
