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

#ifndef SourceListModel_H
#define SourceListModel_H

#include <QAbstractListModel>
#include <QSortFilterProxyModel>

class SourceListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)

    public:
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
        void handleCountChange();

    private:
        QList<QPair<QString, QObject*> > m_sourceList;
};

class SourceListFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)

    public:
        SourceListFilterModel(QObject *parent = 0);
        ~SourceListFilterModel();

        int count() const;

        Q_INVOKABLE void appendSource(const QString& name, QObject* model);
        Q_INVOKABLE void insertSource(int index, const QString& name, QObject* model);
        Q_INVOKABLE QObject *modelForRow(int row) const;

    signals:
        void countChanged();

    protected:
        bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

    private:
        SourceListModel* m_sourceListModel;
};

#endif
