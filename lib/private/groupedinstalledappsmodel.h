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
    explicit GroupedInstalledAppsModel(QObject *parent = 0);
    ~GroupedInstalledAppsModel();

    void init(AbstractSourceRegistry *registry);

    int rowCount(const QModelIndex &parent = QModelIndex()) const; // reimp
    QVariant data(const QModelIndex &, int role = Qt::DisplayRole) const; // reimp

    Q_INVOKABLE QObject *modelForRow(int row) const;

    void setInstaller(const QString &installer);
    QString installer() const;

Q_SIGNALS:
    void installerChanged(const QString &);

private Q_SLOTS:
    void loadRootEntries();
    void loadNextGroup();

private:
    QList<KServiceGroup::Ptr> m_pendingGroupList;
    QList<InstalledAppsModel *> m_models;
    AbstractSourceRegistry *m_registry;

    InstalledAppsModel *createInstalledAppsModel(KServiceGroup::Ptr group);
};

class GroupedInstalledAppsSource : public AbstractSource
{
public:
    GroupedInstalledAppsSource(QObject *parent);
    QAbstractItemModel *createModel(const SourceArguments &/*args*/);
};

} // namespace Homerun

#endif /* GROUPEDINSTALLEDAPPSMODEL_H */
