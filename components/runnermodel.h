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
#ifndef RUNNERMODEL_H
#define RUNNERMODEL_H

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

class RunnerSubModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    explicit RunnerSubModel(const QString &runnerId, const QString &name, QObject *parent = 0);

    enum {
        FavoriteIdRole = Qt::UserRole + 1
    };

    QString runnerId() const { return m_runnerId; }
    QString name() const { return m_name; }

    void setMatches(const QList<Plasma::QueryMatch> &matches);

    int count() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE bool trigger(int row);

Q_SIGNALS:
    void countChanged();

    void triggerRequested(const Plasma::QueryMatch&);

private:
    QString m_runnerId;
    QString m_name;

    QList<Plasma::QueryMatch> m_matches;
};

/**
 *
 */
class RunnerModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString arguments READ arguments WRITE setArguments NOTIFY argumentsChanged)

    /**
     * @property string set the KRunner query
     */
    Q_PROPERTY(QString query WRITE scheduleQuery READ currentQuery NOTIFY queryChanged)

    Q_PROPERTY(bool running READ running NOTIFY runningChanged)

public:
    explicit RunnerModel(QObject *parent = 0);
    ~RunnerModel();

    Q_INVOKABLE QObject *modelForRow(int row) const;

    QString arguments() const;
    void setArguments(const QString &args);

    int rowCount(const QModelIndex &parent = QModelIndex()) const; // reimp
    QVariant data(const QModelIndex &, int role = Qt::DisplayRole) const; // reimp

    bool running() const;

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
    void trigger(const Plasma::QueryMatch&);

private:
    void createManager();

    Plasma::RunnerManager *m_manager;
    QTimer *m_startQueryTimer;
    QTimer *m_runningChangedTimeout;

    QList<RunnerSubModel *> m_models;
    QStringList m_pendingRunnersList;
    bool m_running;
    QString m_pendingQuery;
};

#endif /* RUNNERMODEL_H */
