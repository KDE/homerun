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

// KDE
#include <KServiceGroup>

namespace Homerun {

class InstalledAppsModel;

/**
 * A model which returns all services in grouped sub-models
 */
class GroupedInstalledAppsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit GroupedInstalledAppsModel(const QString &installer, QObject *parent = 0);
    ~GroupedInstalledAppsModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const; // reimp
    QVariant data(const QModelIndex &, int role = Qt::DisplayRole) const; // reimp

    Q_INVOKABLE QObject *modelForRow(int row) const;

Q_SIGNALS:
    void installerChanged(const QString &);
    void applicationLaunched(const QString &);

public Q_SLOTS:
    void refresh(bool reload = true);

private Q_SLOTS:
    void loadRootEntries();
    void loadNextGroup();

private:
    QString m_installer;
    QList<KServiceGroup::Ptr> m_pendingGroupList;
    QList<InstalledAppsModel *> m_models;

    InstalledAppsModel *createInstalledAppsModel(KServiceGroup::Ptr group);
};

class GroupedInstalledAppsSource : public AbstractSource
{
public:
    GroupedInstalledAppsSource(QObject *parent);
    QAbstractItemModel *createModelFromConfigGroup(const KConfigGroup &);
};

} // namespace Homerun

#endif /* GROUPEDINSTALLEDAPPSMODEL_H */
