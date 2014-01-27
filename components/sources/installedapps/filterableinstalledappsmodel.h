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
#ifndef FILTERABLEINSTALLEDAPPSMODEL_H
#define FILTERABLEINSTALLEDAPPSMODEL_H

// Local
#include <abstractsource.h>

// Qt
#include <QSortFilterProxyModel>

// KDE
#include <KServiceGroup>

namespace Plasma {
    class Containment;
}

namespace Homerun {

class InstalledAppsModel;
class FilterableInstalledAppsModel;

class InstalledAppsFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(bool hidden READ hidden WRITE setHidden NOTIFY hiddenChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QObject* containment READ containment WRITE setContainment)

public:
    InstalledAppsFilterModel(const QString &entryPath, const QString &installer, QObject *parent = 0);
    ~InstalledAppsFilterModel();

    int count() const;

    Q_INVOKABLE bool trigger(int row, const QString &actionId = QString(), const QVariant &actionArgument = QVariant());

    QObject *containment() const;
    void setContainment(QObject *containment);

    QString name() const;

    bool hidden() const;
    void setHidden(bool hidden);

Q_SIGNALS:
    void countChanged();
    void hiddenChanged();
    void applicationLaunched(const QString& storageId);
    void addToDesktop(const QString& storageId);
    void addToPanel(const QString& storageId);

public Q_SLOTS:
    void refresh(bool reload = true);

private:
    InstalledAppsModel *m_installedAppsModel;
    bool m_hidden;
};

class SideBarModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit SideBarModel(FilterableInstalledAppsModel *parent = 0);
    ~SideBarModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE void trigger(int row);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
    FilterableInstalledAppsModel *m_sourceModel;
    int m_activeSourceRow;

    friend class FilterableInstalledAppsModel;
};

class FilterableInstalledAppsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString query WRITE scheduleQuery READ currentQuery NOTIFY queryChanged)
    Q_PROPERTY(QObject* sideBarModel READ sideBarModel CONSTANT)

public:
    explicit FilterableInstalledAppsModel(const QString &installer, QObject *parent = 0);
    ~FilterableInstalledAppsModel();

    int count() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const; // reimp
    QVariant data(const QModelIndex &, int role = Qt::DisplayRole) const; // reimp

    QString currentQuery() const;

    SideBarModel *sideBarModel() const;

    Q_INVOKABLE QObject *modelForRow(int row) const;

Q_SIGNALS:
    void countChanged();
    void installerChanged(const QString &);
    void queryChanged(const QString &);
    void applicationLaunched(const QString &);

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
    SideBarModel *m_sideBarModel;
    QString m_query;

    InstalledAppsFilterModel *createInstalledAppsFilterModel(KServiceGroup::Ptr group);
};

class FilterableInstalledAppsSource : public AbstractSource
{
public:
    FilterableInstalledAppsSource(QObject *parent);
    QAbstractItemModel *createModelFromConfigGroup(const KConfigGroup &);
};

} // namespace Homerun

#endif /* FILTERABLEINSTALLEDAPPSMODEL_H */
