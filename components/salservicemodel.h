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

#ifndef SALSERVICEMODEL_H
#define SALSERVICEMODEL_H

#include <QAbstractListModel>
#include <QStringList>

#include <KIcon>
#include <KService>
#include <KServiceGroup>
#include <KUrl>

class PathModel;
class QTimer;

class SalServiceModel;

class AbstractNode
{
public:
    virtual ~AbstractNode();

    virtual bool trigger() = 0;
    virtual QString favoriteId() const { return QString(); }

    KIcon icon() const { return m_icon; }
    QString name() const { return m_name; }

    static bool lessThan(AbstractNode *n1, AbstractNode *n2);

protected:
    QString m_sortKey;
    KIcon m_icon;
    QString m_name;
};

class GroupNode : public AbstractNode
{
public:
    GroupNode(KServiceGroup::Ptr group, SalServiceModel *model);

    bool trigger(); // reimp

private:
    SalServiceModel *m_model;
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
    SalServiceModel *m_model;
    KServiceGroup::Ptr m_group;
    KService::Ptr m_service;
};

class SalServiceModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QObject* pathModel READ pathModel CONSTANT)
    Q_PROPERTY(QString installer READ installer WRITE setInstaller NOTIFY installerChanged)
    Q_PROPERTY(QString arguments READ arguments WRITE setArguments NOTIFY argumentsChanged)

public:
    enum Roles {
        FavoriteIdRole = Qt::UserRole + 1,
    };

    SalServiceModel(QObject *parent = 0);
    ~SalServiceModel();

    int rowCount(const QModelIndex&) const;
    int count() const;
    QVariant data(const QModelIndex&, int) const;

    void setInstaller(const QString &installer);
    QString installer() const;

    void setArguments(const QString &arguments);
    QString arguments() const;

    PathModel *pathModel() const;

    Q_INVOKABLE bool trigger(int row);

Q_SIGNALS:
    void countChanged();
    void installerChanged(const QString &);
    void argumentsChanged(const QString &);
    void openSourceRequested(const QString &source);

private:
    void loadRootEntries();
    void loadServiceGroup(KServiceGroup::Ptr group);

private:
    PathModel *m_pathModel;
    QList<AbstractNode *> m_nodeList;
    QString m_installer;
    QString m_arguments;

    void setPath(const QString &path);
};

#endif
