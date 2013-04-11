/*
Copyright 2012 Aurélien Gâteau <agateau@kde.org>

Based on RunnerModel class. RunnerModel class copyright:

Copyright 2011 Aaron Seigo <aseigo@kde.org>

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
#ifndef RUNNERMODEL_H
#define RUNNERMODEL_H

// Local
#include <abstractsource.h>
#include <querymatchmodel.h>

// Qt
#include <QAbstractListModel>
#include <QStringList>

// KDE
#include <KConfig>
#include <KConfigGroup>
#include <Plasma/QueryMatch>

class QTimer;

namespace Plasma {
class RunnerManager;
}

namespace Homerun {

class RunnerModel;

class RunnerSubModel : public QueryMatchModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
public:
    RunnerSubModel(const QString &runnerId, const QString &name, Plasma::RunnerManager *manager, QObject *parent = 0);

    QString runnerId() const { return m_runnerId; }
    QString name() const { return m_name; }

private:
    QString m_runnerId;
    QString m_name;
};

/**
 *
 */
class RunnerModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * @property string set the KRunner query
     */
    Q_PROPERTY(QString query WRITE scheduleQuery READ currentQuery NOTIFY queryChanged)

    Q_PROPERTY(bool running READ running NOTIFY runningChanged)

public:
    explicit RunnerModel(const KConfigGroup &group, QObject *parent = 0);
    ~RunnerModel();

    Q_INVOKABLE QObject *modelForRow(int row) const;

    void setAllowedRunners(const QStringList &runners);

    int rowCount(const QModelIndex &parent = QModelIndex()) const; // reimp
    QVariant data(const QModelIndex &, int role = Qt::DisplayRole) const; // reimp

    bool running() const;

    QString currentQuery() const;

    Plasma::RunnerManager *manager() const;

public Q_SLOTS:
    void scheduleQuery(const QString &query);

Q_SIGNALS:
    void queryChanged();
    void runningChanged(bool);

private Q_SLOTS:
    void startQuery();
    void queryHasFinished();
    void matchesChanged(const QList<Plasma::QueryMatch> &matches);

private:
    void createManager();
    void loadRunners();
    void clear();

    KConfigGroup m_configGroup;
    Plasma::RunnerManager *m_manager;
    QTimer *m_startQueryTimer;
    QTimer *m_runningChangedTimeout;

    QList<RunnerSubModel *> m_models;
    QStringList m_pendingRunnersList;
    bool m_running;
    QString m_pendingQuery;
};

class RunnerSource : public AbstractSource
{
public:
    RunnerSource(QObject *parent);
    QAbstractItemModel *createModelFromConfigGroup(const KConfigGroup &group);
    SourceConfigurationWidget *createConfigurationWidget(const KConfigGroup &group);
    bool isConfigurable() const;
};

} // namespace Homerun

#endif /* RUNNERMODEL_H */
