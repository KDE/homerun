/*
 *   Copyright 2010 by Marco MArtin <mart@kde.org>

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

#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QSortFilterProxyModel>

class QTimer;

namespace Plasma
{

class DataModel;


class SortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    /**
     * The source model of this sorting proxy model. It has to inherit QAbstractItemModel (ListModel is not supported)
     */
    Q_PROPERTY(QObject *sourceModel READ sourceModel WRITE setModel NOTIFY sourceModelChanged)

    /**
     * The regular expression for the filter, only items with their filterRole matching filterRegExp will be displayed
     */
    Q_PROPERTY(QString filterRegExp READ filterRegExp WRITE setFilterRegExp NOTIFY filterRegExpChanged)

    /**
     * The role of the sourceModel on which filterRegExp must be applied.
     */
    Q_PROPERTY(QString filterRole READ filterRole WRITE setFilterRole)

    /**
     * The role of the sourceModel that will be used for sorting. if empty the order will be left unaltered
     */
    Q_PROPERTY(QString sortRole READ sortRole WRITE setSortRole)

    /**
     * One of Qt.Ascending or Qt.Descending
     */
    Q_PROPERTY(Qt::SortOrder sortOrder READ sortOrder WRITE setSortOrder)

    /**
     * How many items are in this model
     */
    Q_PROPERTY(int count READ count NOTIFY countChanged)

    friend class DataModel;

public:
    SortFilterModel(QObject* parent=0);
    ~SortFilterModel();

    //FIXME: find a way to make QML understnd QAbstractItemModel
    void setModel(QObject *source);

    void setFilterRegExp(const QString &exp);
    QString filterRegExp() const;

    void setFilterRole(const QString &role);
    QString filterRole() const;

    void setSortRole(const QString &role);
    QString sortRole() const;

    void setSortOrder(const Qt::SortOrder order);

    int count() const {return QSortFilterProxyModel::rowCount();}

    /**
     * Returns the item at index in the list model.
     * This allows the item data to be accessed (but not modified) from JavaScript.
     * It returns an Object with a property for each role.
     *
     * @arg int i the row we want
     */
    Q_INVOKABLE QVariantHash get(int i) const;

    Q_INVOKABLE int mapRowToSource(int i) const;

    Q_INVOKABLE int mapRowFromSource(int i) const;

Q_SIGNALS:
    void countChanged();
    void sourceModelChanged(QObject *);
    void filterRegExpChanged(const QString &);

protected:
    int roleNameToId(const QString &name);

protected Q_SLOTS:
    void syncRoleNames();

private Q_SLOTS:
    void onRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow);
    void onRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow);

private:
    QString m_filterRole;
    QString m_sortRole;
    QHash<QString, int> m_roleIds;
};

}
#endif
