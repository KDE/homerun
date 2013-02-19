/*
Copyright 2013 Aurélien Gâteau <agateau@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
// Self
#include <standarditemmodel.h>

// Local

// KDE

// Qt

namespace Homerun
{

//- StandardItem ------------------------------------------------------------------
StandardItem::StandardItem()
{
}

StandardItem::StandardItem(const QString& text, const QString& iconName)
: QStandardItem(text)
{
    setIconName(iconName);
}

void StandardItem::setIconName(const QString &iconName)
{
    setData(iconName, Qt::DecorationRole);
}

bool StandardItem::trigger(const QString &/*actionId*/, const QVariant &/*actionArgument*/)
{
    return false;
}

//- StandardItemModel -------------------------------------------------------------
StandardItemModel::StandardItemModel(QObject *parent)
: QStandardItemModel(parent)
{
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(countChanged()));
    connect(this, SIGNAL(modelReset()), SIGNAL(countChanged()));
}

QString StandardItemModel::name() const
{
    return m_name;
}

void StandardItemModel::setName(const QString &name)
{
    if (m_name == name) {
        return;
    }
    m_name = name;
    nameChanged();
}

int StandardItemModel::count() const
{
    return rowCount(QModelIndex());
}

bool StandardItemModel::trigger(int row, const QString &actionId, const QVariant &actionArgument)
{
    StandardItem *itm = static_cast<StandardItem *>(item(row));
    Q_ASSERT(itm);
    return itm->trigger(actionId, actionArgument);
}

} // namespace

#include <standarditemmodel.moc>
