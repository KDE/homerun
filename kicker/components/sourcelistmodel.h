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

#ifndef SOURCELISTMODEL_H
#define SOURCELISTMODEL_H

#include <QAbstractListModel>

class SourceListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)

    public:
        enum Roles {
            ModelRole = Qt::UserRole + 1,
            CountRole
        };

        SourceListModel(QObject *parent = 0);
        ~SourceListModel();

        int count() const;

        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

        int rowCount(const QModelIndex& parent = QModelIndex()) const;

        Q_INVOKABLE void appendSource(const QString& name, QObject* model);
        Q_INVOKABLE void insertSource(int index, const QString& name, QObject* model);
        Q_INVOKABLE QObject *modelForRow(int row) const;

    signals:
        void countChanged();

    private slots:
        void handleModelDestruction();

    private:
        QList<QPair<QString, QObject*> > m_sourceList;
};

#endif
