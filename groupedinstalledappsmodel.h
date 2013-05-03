/*
Copyright 2012 Aurélien Gâteau <agateau@kde.org>
Copyright 2013 Eike Hein <hein@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef GROUPEDINSTALLEDAPPSMODEL_H
#define GROUPEDINSTALLEDAPPSMODEL_H

// Local
#include <abstractsource.h>

// Qt
#include <QAbstractListModel>
#include <QSortFilterProxyModel>

// KDE
#include <KServiceGroup>

namespace Homerun {

class InstalledAppsModel;
class GroupedInstalledAppsModel;

class InstalledAppsFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString name READ name CONSTANT)

public:
    InstalledAppsFilterModel(const QString &entryPath, const QString &installer, QObject *parent = 0);
    ~InstalledAppsFilterModel();

    int count() const;

    Q_INVOKABLE bool trigger(int row);

    QString name() const;

Q_SIGNALS:
    void countChanged();

public Q_SLOTS:
    void refresh(bool reload = true);

private:
    InstalledAppsModel *m_installedAppsModel;
};

class FilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int activeFilterIndex READ activeFilterIndex WRITE setActiveFilterIndex NOTIFY activeFilterIndexChanged)

public:
    enum Roles {
        ActiveFilterRole = Qt::UserRole + 1,
    };

    FilterModel(GroupedInstalledAppsModel *sourceModel);
    ~FilterModel();

    QVariant data(const QModelIndex &, int role = Qt::DisplayRole) const; // reimp

    int activeFilterIndex() const;

public Q_SLOTS:
    void setActiveFilterIndex(int index);

Q_SIGNALS:
    void activeFilterIndexChanged();

protected:
    bool filterAcceptsRow (int source_row, const QModelIndex &source_parent ) const;

private:
    GroupedInstalledAppsModel *m_sourceModel;
    int m_activeFilterIndex;

    friend class GroupedInstalledAppsModel;
};

/**
 * A model which returns all services in grouped sub-models
 */
class GroupedInstalledAppsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString query WRITE scheduleQuery READ currentQuery NOTIFY queryChanged)
    Q_PROPERTY(QObject* filterModel READ filterModel CONSTANT)

public:
    explicit GroupedInstalledAppsModel(const QString &installer, QObject *parent = 0);
    ~GroupedInstalledAppsModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const; // reimp
    QVariant data(const QModelIndex &, int role = Qt::DisplayRole) const; // reimp

    Q_INVOKABLE QObject *modelForRow(int row) const;

    QString currentQuery() const;

    FilterModel *filterModel() const;

Q_SIGNALS:
    void installerChanged(const QString &);
    void queryChanged(const QString &);

public Q_SLOTS:
    void refresh(bool reload = true);
    void scheduleQuery(const QString &query);

private Q_SLOTS:
    void loadRootEntries();
    void loadNextGroup();

private:
    QString m_installer;
    QList<KServiceGroup::Ptr> m_pendingGroupList;
    QList<InstalledAppsFilterModel *> m_models;
    FilterModel* m_filterModel;
    QString m_query;

    InstalledAppsFilterModel *createInstalledAppsModel(KServiceGroup::Ptr group);
};

class GroupedInstalledAppsSource : public AbstractSource
{
public:
    GroupedInstalledAppsSource(QObject *parent, const QVariantList &args = QVariantList());
    QAbstractItemModel *createModelFromConfigGroup(const KConfigGroup &);
};

} // namespace Homerun

#endif /* GROUPEDINSTALLEDAPPSMODEL_H */
