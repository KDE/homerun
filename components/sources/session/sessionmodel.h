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

// Qt
#include <QStandardItem>
#include <QStandardItemModel>

// KDE
#include <kworkspace/kdisplaymanager.h>
#include <KService>

class QString;

namespace Homerun {

enum ActionType {
    Logout,
    StartNewSession,
    Lock
};

class StandardItem : public QStandardItem
{
public:
    StandardItem();
    StandardItem(const QString &text, const QString &iconName);
    virtual bool trigger(const QString &actionId, const QVariant &actionArgument);
    void setIconName(const QString &);
};

class StandardItemModel : public QStandardItemModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    StandardItemModel(QObject *parent = 0);

    int count() const;
    QString name() const;
    void setName(const QString &name);

    Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &actionArgument);

Q_SIGNALS:
    void countChanged();
    void nameChanged();

private:
    QString m_name;
};

class SessionModel : public StandardItemModel
{
    Q_OBJECT
public:
    SessionModel(QObject *parent = 0);

private:
    KDisplayManager m_displayManager;
    void createUserItems();
};

} // namespace Homerun

#endif
