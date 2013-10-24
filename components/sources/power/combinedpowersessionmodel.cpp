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

#include "combinedpowersessionmodel.h"
#include "sessionmodel.h"
#include "powermodel.h"

#include <KLocale>

namespace Homerun {

CombinedPowerSessionModel::CombinedPowerSessionModel(QObject* parent) : QAbstractListModel(parent)
, m_sessionModel(new SessionModel(this))
, m_powerModel(new PowerModel(this))
{
    connect(m_sessionModel, SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(countChanged()));
    connect(m_sessionModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(countChanged()));
    connect(m_sessionModel, SIGNAL(modelReset()), SIGNAL(countChanged()));

    connect(m_powerModel, SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(countChanged()));
    connect(m_powerModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(countChanged()));
    connect(m_powerModel, SIGNAL(modelReset()), SIGNAL(countChanged()));
}

CombinedPowerSessionModel::~CombinedPowerSessionModel()
{
}

int CombinedPowerSessionModel::count() const
{
    return rowCount();
}

int CombinedPowerSessionModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : (m_powerModel->count() + m_sessionModel->count());
}

QVariant CombinedPowerSessionModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();

    if (row >= m_sessionModel->count()) {
        return m_powerModel->data(m_powerModel->index(row - m_sessionModel->count(), index.column()), role);
    }

    return m_sessionModel->data(m_sessionModel->index(index.row(), index.column()), role);
}

bool CombinedPowerSessionModel::trigger(int row, const QString&, const QVariant&)
{
    if (row >= m_sessionModel->count()) {
        return m_powerModel->trigger(row - m_sessionModel->count(), QString(), QVariant());
    }

    return m_sessionModel->trigger(row, QString(), QVariant());
}

QString CombinedPowerSessionModel::name() const
{
    return i18n("Power / Session");
}

}

#include "combinedpowersessionmodel.moc"
