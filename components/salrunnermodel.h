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
#ifndef SALRUNNERMODEL_H
#define SALRUNNERMODEL_H

// Local

// Qt
#include <QAbstractListModel>
#include <QStringList>

// KDE
#include <Plasma/QueryMatch>

class QTimer;

namespace Plasma {
class RunnerManager;
}

class SalRunnerSubModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    explicit SalRunnerSubModel(const QString &runnerId, const QString &name, QObject *parent = 0);

    QString runnerId() const { return m_runnerId; }
    QString name() const { return m_name; }

    void setMatches(const QList<Plasma::QueryMatch> &matches);

    int count() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

Q_SIGNALS:
    void countChanged();

private:
    QString m_runnerId;
    QString m_name;

    QList<Plasma::QueryMatch> m_matches;
};

/**
 *
 */
class SalRunnerModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QStringList arguments READ arguments WRITE setArguments NOTIFY argumentsChanged)

    /**
     * @property string set the KRunner query
     */
    Q_PROPERTY(QString query WRITE scheduleQuery READ currentQuery NOTIFY queryChanged)

public:
    explicit SalRunnerModel(QObject *parent = 0);
    ~SalRunnerModel();

    Q_INVOKABLE QObject *modelForRow(int row) const;

    QStringList arguments() const;
    void setArguments(const QStringList &args);

    int rowCount(const QModelIndex &parent = QModelIndex()) const; // reimp
    QVariant data(const QModelIndex &, int role = Qt::DisplayRole) const; // reimp

    QString currentQuery() const;

public Q_SLOTS:
    void scheduleQuery(const QString &query);

Q_SIGNALS:
    void argumentsChanged();
    void queryChanged();
    void runningChanged(bool);

private Q_SLOTS:
    void startQuery();
    void queryHasFinished();
    void matchesChanged(const QList<Plasma::QueryMatch> &matches);

private:
    void createManager();

    Plasma::RunnerManager *m_manager;
    QTimer *m_startQueryTimer;
    QTimer *m_runningChangedTimeout;

    QList<SalRunnerSubModel *> m_models;
    QStringList m_pendingRunnersList;
    bool m_running;
    QString m_pendingQuery;
};

#endif /* SALRUNNERMODEL_H */
