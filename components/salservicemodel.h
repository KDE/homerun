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

class QTimer;

struct Node
{
    KIcon icon;
    QString name;
    QString entryPath;
    KService::Ptr service;

    static Node fromService(KService::Ptr);
    static Node fromServiceGroup(KServiceGroup::Ptr);
};

class SalServiceModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString path WRITE setPath READ path NOTIFY pathChanged)

public:
    enum Roles {
        EntryPathRole = Qt::UserRole + 1,
    };

    SalServiceModel (QObject *parent = 0);

    int rowCount(const QModelIndex&) const;
    int count() const;
    QVariant data(const QModelIndex&, int) const;

    void setPath(const QString& path);
    QString path() const;

    Q_INVOKABLE bool trigger(int row);

Q_SIGNALS:
    void countChanged();
    void pathChanged(const QString&);

private:
    void loadRootEntries();
    void loadServiceGroup(KServiceGroup::Ptr group);

private:
    QList<Node> m_nodeList;
    QString m_path;
};

#endif
