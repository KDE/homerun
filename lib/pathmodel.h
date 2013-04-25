/*
Copyright 2012 Aurélien Gâteau <agateau@kde.org>

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
#ifndef PATHMODEL_H
#define PATHMODEL_H

// Local
#include <homerun_export.h>

// Qt
#include <QStandardItemModel>

// KDE

namespace Homerun {

class PathModelPrivate;

/**
 * Represents the path in a browsable Homerun model
 */
class HOMERUN_EXPORT PathModel : public QStandardItemModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    explicit PathModel(QObject *parent = 0);
    ~PathModel();

    void addPath(const QString &label, const QString &sourceId, const QVariantMap &sourceArguments);

    int count() const;

    Q_INVOKABLE QVariantHash get(int row) const;

Q_SIGNALS:
    void countChanged();

private:
    PathModelPrivate * const d;
};

} // namespace Homerun

#endif /* PATHMODEL_H */
