/*
 *   Copyright 2010 by Marco Martin <mart@kde.org>

 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "datamodel.h"

#include <QTimer>

#include <KDebug>

namespace Plasma
{

SortFilterModel::SortFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    setObjectName("SortFilterModel");
    setDynamicSortFilter(true);
    connect(this, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            this, SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
            this, SIGNAL(countChanged()));
    connect(this, SIGNAL(modelReset()),
            this, SIGNAL(countChanged()));
}

SortFilterModel::~SortFilterModel()
{
}

void SortFilterModel::syncRoleNames()
{
    m_roleIds.clear();

    setRoleNames(sourceModel()->roleNames());
    QHash<int, QByteArray>::const_iterator i;
    for (i = roleNames().constBegin(); i != roleNames().constEnd(); ++i) {
        m_roleIds[i.value()] = i.key();
    }
    setFilterRole(m_filterRole);
    setSortRole(m_sortRole);
}

int SortFilterModel::roleNameToId(const QString &name)
{
    if (!m_roleIds.contains(name)) {
        return Qt::DisplayRole;
    }
    return m_roleIds.value(name);
}

void SortFilterModel::setModel(QObject *source)
{
    if (source == sourceModel()) {
        return;
    }

    QAbstractItemModel *model = 0;
    if (source) {
        model = qobject_cast<QAbstractItemModel *>(source);
        if (!model) {
            kWarning() << "Error: QAbstractItemModel type expected";
            return;
        }
    }

    if (sourceModel()) {
        disconnect(sourceModel(), SIGNAL(modelReset()), this, SLOT(syncRoleNames()));
        disconnect(sourceModel(), SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(onRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
        disconnect(sourceModel(), SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(onRowsMoved(QModelIndex,int,int,QModelIndex,int)));
    }
    if (model) {
        connect(model, SIGNAL(modelReset()), this, SLOT(syncRoleNames()));
        connect(model, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(onRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
        connect(model, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(onRowsMoved(QModelIndex,int,int,QModelIndex,int)));
    }
    QSortFilterProxyModel::setSourceModel(model);
    sourceModelChanged(model);
}




void SortFilterModel::setFilterRegExp(const QString &exp)
{
    if (exp == filterRegExp()) {
        return;
    }
    QSortFilterProxyModel::setFilterRegExp(QRegExp(exp, Qt::CaseInsensitive));
    filterRegExpChanged(exp);
}

QString SortFilterModel::filterRegExp() const
{
    return QSortFilterProxyModel::filterRegExp().pattern();
}

void SortFilterModel::setFilterRole(const QString &role)
{
    QSortFilterProxyModel::setFilterRole(roleNameToId(role));
    m_filterRole = role;
}

QString SortFilterModel::filterRole() const
{
    return m_filterRole;
}

void SortFilterModel::setSortRole(const QString &role)
{
    if (m_sortRole == role) {
        return;
    }
    m_sortRole = role;
    if (role.isEmpty()) {
        sort(-1, Qt::AscendingOrder);
    } else {
        QSortFilterProxyModel::setSortRole(roleNameToId(role));
        sort(0, sortOrder());
    }
}

QString SortFilterModel::sortRole() const
{
    return m_sortRole;
}

void SortFilterModel::setSortOrder(const Qt::SortOrder order)
{
    sort(0, order);
}

QVariantHash SortFilterModel::get(int row) const
{
    QModelIndex idx = index(row, 0);
    QVariantHash hash;

    QHash<int, QByteArray>::const_iterator i;
    for (i = roleNames().constBegin(); i != roleNames().constEnd(); ++i) {
        hash[i.value()] = data(idx, i.key());
    }

    return hash;
}

int SortFilterModel::mapRowToSource(int row) const
{
    QModelIndex idx = index(row, 0);
    return mapToSource(idx).row();
}

int SortFilterModel::mapRowFromSource(int row) const
{
    if (!sourceModel()) {
        kWarning() << "No source model defined!";
        return -1;
    }
    QModelIndex idx = sourceModel()->index(row, 0);
    return mapFromSource(idx).row();
}

void SortFilterModel::onRowsAboutToBeMoved(const QModelIndex &fromParent, int sourceStart, int sourceEnd, const QModelIndex &toParent, int sourceTo)
{
    if (fromParent.isValid() || toParent.isValid()) {
        kWarning() << "Only flat models are supported";
        return;
    }
    int start = mapRowFromSource(sourceStart);
    int end = mapRowFromSource(sourceEnd);
    int to;
    if (sourceStart < sourceTo) {
        // Compensate for the +1 when moving a row down (see QAbstractItemModel::beginMoveRows doc)
        to = mapRowFromSource(sourceTo - 1) + 1;
        Q_ASSERT(to >= 1);
    } else {
        to = mapRowFromSource(sourceTo);
        Q_ASSERT(to >= 0);
    }
    beginMoveRows(
        QModelIndex(), start, end,
        QModelIndex(), to);
}

void SortFilterModel::onRowsMoved(const QModelIndex &fromParent, int /*fromStart*/, int /*fromEnd*/, const QModelIndex &toParent, int /*toRow*/)
{
    if (fromParent.isValid() || toParent.isValid()) {
        kWarning() << "Only flat models are supported";
        return;
    }
    endMoveRows();
}

}

#include "datamodel.moc"
