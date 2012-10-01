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
#ifndef CLOCKS_H
#define CLOCKS_H

// KDE
#include <QStandardItemModel>

/**
 * This model exposes recent documents handled by the RecentDocument class
 */
class ClocksModel : public QStandardItemModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    ClocksModel();

    Q_INVOKABLE bool trigger(int row);

    QString name() const;

    int count() const;

Q_SIGNALS:
    void countChanged();

private:
    void load();
    void scheduleUpdate();

private Q_SLOTS:
    void update();
};

#endif /* CLOCKS_H */
