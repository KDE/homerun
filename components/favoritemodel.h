/*
    Copyright 2009 Ivan Cukic <ivan.cukic+kde@gmail.com>
    Copyright 2010 Marco Martin <notmart@gmail.com>

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

#ifndef FAVORITEMODEL_H
#define FAVORITEMODEL_H

#include <QAbstractListModel>

#include <KService>
#include <KSharedConfig>

class FavoriteModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString configFileName READ configFileName WRITE setConfigFileName NOTIFY configFileNameChanged)

public:
    enum Roles {
        UrlRole = Qt::UserRole + 1,
    };

    FavoriteModel(QObject *parent = 0);
    ~FavoriteModel();

    int count() const;

    QString configFileName() const;
    void setConfigFileName(const QString &name);

    void setConfig(const KSharedConfig::Ptr &);

    int rowCount(const QModelIndex &) const;
    QVariant data(const QModelIndex &, int role) const;

    Q_INVOKABLE void add(const QString &entryPath);
    Q_INVOKABLE void remove(const QString &entryPath);
    Q_INVOKABLE bool isFavorite(const QString &entryPath) const;

    Q_INVOKABLE void run(int row);

Q_SIGNALS:
    void countChanged();
    void configFileNameChanged();

private:
    KSharedConfig::Ptr m_config;
    QList<KService::Ptr> m_favoriteList;

    int indexOfByPath(const QString &) const;
};

#endif // FAVORITEMODEL_H
