/*
    Copyright 2011 Aaron Seigo <aseigo@kde.org>
    Copyright (C) 2012 Shaun Reich <shaun.reich@blue-systems.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef INSTALLEDAPPSMODEL_H
#define INSTALLEDAPPSMODEL_H

// Local
#include <abstractsource.h>

// Qt
#include <QAbstractListModel>
#include <QStringList>

// KDE
#include <KIcon>
#include <KService>
#include <KServiceGroup>
#include <KUrl>

namespace Homerun {

class PathModel;

class InstalledAppsModel;
class InstalledAppsSource;

class AbstractNode
{
public:
    virtual ~AbstractNode();

    virtual bool trigger() = 0;
    virtual QString favoriteId() const { return QString(); }

    QString icon() const { return m_icon; }
    QString name() const { return m_name; }

    static bool lessThan(AbstractNode *n1, AbstractNode *n2);

protected:
    QString m_sortKey;
    QString m_icon;
    QString m_name;
};

class GroupNode : public AbstractNode
{
public:
    GroupNode(KServiceGroup::Ptr group, InstalledAppsModel *model);

    bool trigger(); // reimp

private:
    InstalledAppsModel *m_model;
    QString m_entryPath;
};

class AppNode : public AbstractNode
{
public:
    AppNode(KService::Ptr service);

    bool trigger(); // reimp;
    QString favoriteId() const; // reimp

private:
    KService::Ptr m_service;
};

class InstallerNode : public AbstractNode
{
public:
    InstallerNode(KServiceGroup::Ptr group, KService::Ptr installerService);

    bool trigger(); // reimp

private:
    InstalledAppsModel *m_model;
    KServiceGroup::Ptr m_group;
    KService::Ptr m_service;
};

class InstalledAppsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QObject* pathModel READ pathModel CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)

public:
    enum Roles {
        FavoriteIdRole = Qt::UserRole + 1,
    };

    InstalledAppsModel(const QString &entryPath, const QString &installer, QObject *parent = 0);
    ~InstalledAppsModel();

    int rowCount(const QModelIndex&) const;
    int count() const;
    QVariant data(const QModelIndex&, int) const;

    PathModel *pathModel() const;

    Q_INVOKABLE bool trigger(int row);

    QString name() const;

Q_SIGNALS:
    void countChanged();
    void openSourceRequested(const QString &sourceId, const QVariantMap &args);

private:
    void loadRootEntries();
    void loadServiceGroup(KServiceGroup::Ptr group);
    void doLoadServiceGroup(KServiceGroup::Ptr group);

    PathModel *m_pathModel;
    QList<AbstractNode *> m_nodeList;
    QString m_installer;
    QString m_arguments;

    void load(const QString &entryPath);

    friend class GroupNode;
};

class InstalledAppsSource : public AbstractSource
{
public:
    InstalledAppsSource(QObject *parent);
    QAbstractItemModel *createModelFromConfigGroup(const KConfigGroup &group); // reimp
    QAbstractItemModel *createModelFromArguments(const QVariantMap &arguments); // reimp

    bool isConfigurable() const; // reimp
    SourceConfigurationWidget *createConfigurationWidget(const KConfigGroup &group); // reimp

private:
    QAbstractItemModel *createModel(const QString &entryPath);
};

} // namespace Homerun

#endif
