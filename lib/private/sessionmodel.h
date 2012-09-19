/*
    Copyright 2012 Aurélien Gâteau <agateau@kde.org>
    Copyright 2012 (C) Shaun Reich <shaun.reich@blue-systems.com>

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

#ifndef SESSIONMODEL_H
#define SESSIONMODEL_H

#include <QAbstractListModel>

#include <KService>

class QString;

namespace Homerun {

enum ActionType {
    Logout,
    SwitchUser,
    Lock
};

struct SessionAction
{
    QString name;
    QString iconName;
    ActionType type;
};

class SessionModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    SessionModel(QObject *parent = 0);
    ~SessionModel();

    int count() const;
    QString name() const;

    int rowCount(const QModelIndex & = QModelIndex()) const;
    QVariant data(const QModelIndex &, int role = Qt::DisplayRole) const;

    Q_INVOKABLE bool trigger(int row);

Q_SIGNALS:
    void countChanged();

private:
    QList<SessionAction> m_sessionList;
};

} // namespace Homerun

#endif
