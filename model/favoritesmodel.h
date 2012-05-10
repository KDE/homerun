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

#ifndef FAVOURITESMODEL_H
#define FAVOURITESMODEL_H


#include <KConfigGroup>

#include <Plasma/QueryMatch>

#include <QStandardItemModel>
#include <QStringList>
#include <KService>
#include <KServiceGroup>
#include <KUrl>
#include <QMimeData>

namespace Plasma {
    class RunnerManager;
}

class QTimer;

class FavoritesModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        Url = Qt::UserRole + 1,
    };

    FavoritesModel (QObject *parent = 0);

    int rowCount(const QModelIndex&) const;
    int count() const;
    QVariant data(const QModelIndex&, int) const;

    void restore(KConfigGroup &cg);

Q_SIGNALS:
    void countChanged();
};

#endif // FAVOURITESMODEL_H

