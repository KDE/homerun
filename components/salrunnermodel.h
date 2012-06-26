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
#ifndef SALRUNNERMODEL_H
#define SALRUNNERMODEL_H

// Local

// Qt
#include <QAbstractListModel>
#include <QStringList>

// KDE

class QStandardItemModel;

/**
 *
 */
class SalRunnerModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QStringList arguments READ arguments WRITE setArguments NOTIFY argumentsChanged)
public:
    explicit SalRunnerModel(QObject *parent = 0);
    ~SalRunnerModel();

    Q_INVOKABLE QObject *modelForRow(int row) const;

    QStringList arguments() const;
    void setArguments(const QStringList &args);

    int rowCount(const QModelIndex &) const; // reimp
    QVariant data(const QModelIndex &, int role = Qt::DisplayRole) const; // reimp

Q_SIGNALS:
    void argumentsChanged();

private:
    QList<QStandardItemModel *> m_models;
    QStringList m_arguments;
};

#endif /* SALRUNNERMODEL_H */
