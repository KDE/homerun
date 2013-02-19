/*
Copyright 2013 Aurélien Gâteau <agateau@kde.org>

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
#ifndef STANDARDITEMMODEL_H
#define STANDARDITEMMODEL_H

// Local

// Qt
#include <QStandardItemModel>

// KDE

namespace Homerun
{

/**
 * An item to be used with StandardItemModel. Similar to QStandardItem, but
 * more adapted to Homerun needs.
 */
class StandardItem : public QStandardItem
{
public:
    StandardItem();
    StandardItem(const QString &text, const QString &iconName);
    virtual bool trigger(const QString &actionId, const QVariant &actionArgument);
    void setIconName(const QString &);
};

/**
 * A simple model to use when a source does not have many items. Similar to
 * QStandardItemModel, but more adapted to Homerun needs.
 *
 * Important: One should always use items inheriting from StandardItem with this
 * model.
 */
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

    Q_INVOKABLE virtual bool trigger(int row, const QString &actionId, const QVariant &actionArgument);

Q_SIGNALS:
    void countChanged();
    void nameChanged();

private:
    QString m_name;
};

} // namespace


#endif /* STANDARDITEMMODEL_H */
