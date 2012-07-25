/*
 * Copyright 2012 Aurélien Gâteau <agateau@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef PATHMODEL_H
#define PATHMODEL_H

// Local

// Qt
#include <QStandardItemModel>

// KDE

/**
 * Represents the path in a browsable SAL model
 */
class PathModel : public QStandardItemModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    explicit PathModel(QObject *parent = 0);
    ~PathModel();

    enum {
        SourceRole = Qt::UserRole + 1,
    };

    void addPath(const QString &label, const QString &source);

    int count() const;

Q_SIGNALS:
    void countChanged();
};

#endif /* PATHMODEL_H */
