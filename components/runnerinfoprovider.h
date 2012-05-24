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
#ifndef RUNNERINFOPROVIDER_H
#define RUNNERINFOPROVIDER_H

// Qt
#include <QObject>

// KDE
#include <KPluginInfo>

/**
 * Provides information (name, icon...) about a runner, given its runnerId
 */
class RunnerInfoProvider : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString runnerId READ runnerId WRITE setRunnerId NOTIFY runnerIdChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
public:
    explicit RunnerInfoProvider(QObject *parent = 0);
    ~RunnerInfoProvider();

    QString runnerId() const;
    void setRunnerId(const QString &runnerId);

    QString name() const;

    QString icon() const;

Q_SIGNALS:
    void runnerIdChanged();
    void nameChanged();
    void iconChanged();

private:
    QString m_runnerId;
    KPluginInfo m_info;
};

#endif /* RUNNERINFOPROVIDER_H */
