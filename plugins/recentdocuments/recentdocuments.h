/*
Copyright 2012 Aurélien Gâteau <agateau@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef RECENTDOCUMENTS_H
#define RECENTDOCUMENTS_H

// KDE
#include <QStandardItemModel>

/**
 * This model exposes recent documents handled by the RecentDocument class
 */
class RecentDocumentsModel : public QStandardItemModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    RecentDocumentsModel();

    enum {
        UrlRole = Qt::UserRole + 1,
        DesktopPathRole,
        HasActionListRole,
        ActionListRole,
    };

    Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &actionArgument);

    QString name() const;

    int count() const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

Q_SIGNALS:
    void countChanged();

private Q_SLOTS:
    void load();

private:
    void forget(QStandardItem *item);
};

#endif /* RECENTDOCUMENTS_H */
