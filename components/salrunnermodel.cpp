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
#include <salrunnermodel.h>

// Local

// KDE
#include <KDebug>

// Qt
#include <QStandardItemModel>

class StandardItemModel : public QStandardItemModel
{
public:
    explicit StandardItemModel(QObject *parent = 0)
    : QStandardItemModel(parent)
    {
        QHash<int, QByteArray> roles;
        roles.insert(Qt::DisplayRole, "label");
        roles.insert(Qt::DecorationRole, "icon");
        setRoleNames(roles);
    }
};

SalRunnerModel::SalRunnerModel(QObject *parent)
: QAbstractListModel(parent)
{
    kWarning();
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "label");
    roles.insert(Qt::DecorationRole, "icon");
    setRoleNames(roles);

    StandardItemModel *m;
    m = new StandardItemModel(this);
    m->setProperty("name", "A");
    m->appendRow(new QStandardItem("A1"));
    m->appendRow(new QStandardItem("A2"));
    m->appendRow(new QStandardItem("A3"));
    m_models << m;
    m = new StandardItemModel(this);
    m->setProperty("name", "B");
    m->appendRow(new QStandardItem("B1"));
    m->appendRow(new QStandardItem("B2"));
    m_models << m;
}

SalRunnerModel::~SalRunnerModel()
{
    kWarning();
}

int SalRunnerModel::rowCount(const QModelIndex &parent) const
{
    kWarning() << parent;
    if (parent.isValid()) {
        return 0;
    }
    return m_models.count();
}

QVariant SalRunnerModel::data(const QModelIndex &index, int role) const
{
    kWarning() << index << role;
    if (index.parent().isValid()) {
        return QVariant();
    }

    QStandardItemModel *model = m_models.value(index.row());
    if (!model) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        kWarning() << model->property("name");
        return model->property("name");
    } else {
        return QVariant();
    }
}

QObject *SalRunnerModel::modelForRow(int row) const
{
    kWarning() << row;
    return m_models.value(row);
}

#include <salrunnermodel.moc>
