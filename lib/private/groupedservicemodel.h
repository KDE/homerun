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
#ifndef GROUPEDSERVICEMODEL_H
#define GROUPEDSERVICEMODEL_H

// Local
#include <abstractsource.h>

// Qt
#include <QAbstractListModel>

// KDE
#include <KServiceGroup>

namespace Homerun {

class ServiceModel;
class SourceRegistry;

/**
 * A model which returns all services in grouped sub-models
 */
class GroupedServiceModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit GroupedServiceModel(QObject *parent = 0);
    ~GroupedServiceModel();

    void init(SourceRegistry *registry);

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
    QList<ServiceModel *> m_models;
    SourceRegistry *m_registry;

    ServiceModel *createServiceModel(KServiceGroup::Ptr group);
};

class GroupedServiceSource : public AbstractSource
{
public:
    GroupedServiceSource(SourceRegistry *registry);
    QAbstractItemModel *createModel(const SourceArguments &/*args*/);
};

} // namespace Homerun

#endif /* GROUPEDSERVICEMODEL_H */
