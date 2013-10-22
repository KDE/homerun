/*
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
#ifndef RECENTAPPSMODEL_H
#define RECENTAPPSMODEL_H

// Local
#include <abstractsource.h>

// Qt
#include <QAbstractListModel>

// KDE
#include <KConfigGroup>

namespace Homerun {

class RecentAppsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QObject* containment READ containment WRITE setContainment)

    public:
        enum Roles {
            FavoriteIdRole = Qt::UserRole + 1,
            HasActionListRole,
            ActionListRole
        };

        RecentAppsModel(const KConfigGroup &group, QObject *parent = 0);
        ~RecentAppsModel();

        int rowCount(const QModelIndex &parent = QModelIndex()) const;
        int count() const;
        QVariant data(const QModelIndex &, int) const;

        Q_INVOKABLE void addApp(const QString &storageId, bool sync = true);
        Q_INVOKABLE bool forgetApp(int row, bool sync = true);

        Q_INVOKABLE bool trigger(int row, const QString &actionId = QString(), const QVariant &actionArgument = QVariant());

        QObject *containment() const;
        void setContainment(QObject *containment);

        QString name() const;

    Q_SIGNALS:
        void countChanged();

    private:
        QList<QString> m_storageIdList;
        KConfigGroup m_configGroup;

        QObject *m_containment;
};

class RecentAppsSource : public AbstractSource
{
public:
    RecentAppsSource(QObject *parent);
    QAbstractItemModel *createModelFromConfigGroup(const KConfigGroup &group); // reiml
};

} // namespace Homerun

#endif
