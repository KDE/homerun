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
// Self
#include <runnermodel.h>

// Local
#include <sourceid.h>

// KDE
#include <KDebug>
#include <Plasma/AbstractRunner>
#include <Plasma/RunnerManager>

// Qt
#include <QStandardItemModel>
#include <QTimer>

namespace Homerun {

RunnerSubModel::RunnerSubModel(const QString &runnerId, const QString &name, QObject *parent)
: QAbstractListModel(parent)
, m_runnerId(runnerId)
, m_name(name)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::DecorationRole, "icon");
    roles.insert(FavoriteIdRole, "favoriteId");
    /*
    roles.insert(Type, "type");
    roles.insert(Relevance, "relevance");
    roles.insert(Data, "data");
    roles.insert(Id, "id");
    roles.insert(SubText, "description");
    roles.insert(Enabled, "enabled");
    roles.insert(RunnerId, "runnerid");
    roles.insert(RunnerName, "runnerName");
    roles.insert(Actions, "actions");
    */
    setRoleNames(roles);
}

int RunnerSubModel::count() const
{
    return m_matches.count();
}

int RunnerSubModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_matches.count();
}

QVariant RunnerSubModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.parent().isValid() ||
        index.column() > 0 || index.row() < 0 || index.row() >= m_matches.count()) {
        // index requested must be valid, but we have no child items!
        //kDebug() << "invalid index requested";
        return QVariant();
    }

    const Plasma::QueryMatch &match = m_matches.at(index.row());
    if (role == Qt::DisplayRole) {
        return match.text();
    } else if (role == Qt::DecorationRole) {
        return match.icon();
    } else if (role == FavoriteIdRole) {
        QString runnerId = match.runner()->id();
        if (runnerId == "services") {
            return QVariant("app:" + match.data().toString());
        } else if (runnerId == "locations") {
            KUrl url(match.data().toString());
            return QVariant("place:" + url.url());
        } else {
            return QString();
        }
    } /*else if (role == Type) {
        return m_matches.at(index.row()).type();
    } else if (role == Relevance) {
        return m_matches.at(index.row()).relevance();
    } else if (role == Data) {
        return m_matches.at(index.row()).data();
    } else if (role == Id) {
        return m_matches.at(index.row()).id();
    } else if (role == SubText) {
        return m_matches.at(index.row()).subtext();
    } else if (role == Enabled) {
        return m_matches.at(index.row()).isEnabled();
    } else if (role == RunnerId) {
        return m_matches.at(index.row()).runner()->id();
    } else if (role == RunnerName) {
        return m_matches.at(index.row()).runner()->name();
    } else if (role == Actions) {
        QVariantList actions;
        Plasma::QueryMatch amatch = m_matches.at(index.row());
        QList<QAction*> theactions = m_manager->actionsForMatch(amatch);
        foreach(QAction* action, theactions) {
            actions += qVariantFromValue<QObject*>(action);
        }
        return actions;
    }*/

    return QVariant();
}

void RunnerSubModel::setMatches(const QList<Plasma::QueryMatch> &matches)
{
    bool fullReset = false;
    int oldCount = m_matches.count();
    int newCount = matches.count();
    if (newCount > oldCount) {
        // We received more matches than we had. If all common matches are the
        // same, we can just append new matches instead of resetting the whole
        // model
        for (int row = 0; row < oldCount; ++row) {
            if (!(m_matches.at(row) == matches.at(row))) {
                fullReset = true;
                break;
            }
        }
        if (!fullReset) {
            // Not a full reset, inserting rows
            beginInsertRows(QModelIndex(), oldCount, newCount);
            m_matches = matches;
            endInsertRows();
            emit countChanged();
        }
    } else {
        fullReset = true;
    }

    if (fullReset) {
        beginResetModel();
        m_matches = matches;
        endResetModel();
        emit countChanged();
    }
}

bool RunnerSubModel::trigger(int row)
{
    if (row >= 0 && row < m_matches.count()) {
        triggerRequested(m_matches.at(row));
    }
    return true;
}

//--------------------------------------------------------------------

RunnerModel::RunnerModel(QObject *parent)
: QAbstractListModel(parent)
, m_manager(0)
, m_startQueryTimer(new QTimer(this))
, m_runningChangedTimeout(new QTimer(this))
, m_running(false)
{
    m_startQueryTimer->setSingleShot(true);
    m_startQueryTimer->setInterval(10);
    connect(m_startQueryTimer, SIGNAL(timeout()), this, SLOT(startQuery()));

    //FIXME: HACK: some runners stay in a running but finished state, not possible to say if it's actually over
    m_runningChangedTimeout->setSingleShot(true);
    connect(m_runningChangedTimeout, SIGNAL(timeout()), this, SLOT(queryHasFinished()));
}

RunnerModel::~RunnerModel()
{
}

int RunnerModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_models.count();
}

QVariant RunnerModel::data(const QModelIndex &index, int role) const
{
    if (index.parent().isValid()) {
        return QVariant();
    }

    RunnerSubModel *model = m_models.value(index.row());
    if (!model) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return model->name();
    } else {
        return QVariant();
    }
}

bool RunnerModel::running() const
{
    return m_running;
}

QObject *RunnerModel::modelForRow(int row) const
{
    if (row < 0 || row >= m_models.count()) {
        kWarning() << "No model for row" << row << "!";
        return 0;
    }
    return m_models.value(row);
}

void RunnerModel::setAllowedRunners(const QStringList &list)
{
    QStringList existingList = m_manager ? m_manager->allowedRunners() : m_pendingRunnersList;

    if (existingList.toSet() == list.toSet()) {
        return;
    }
    if (m_manager) {
        m_manager->setAllowedRunners(list);

        //automagically enter single runner mode if there's only 1 allowed runner
        m_manager->setSingleMode(list.count() == 1);
    } else {
        m_pendingRunnersList = list;
    }
}

QString RunnerModel::currentQuery() const
{
    return m_manager ? m_manager->query() : QString();
}

void RunnerModel::scheduleQuery(const QString &query)
{
    m_pendingQuery = query;
    m_startQueryTimer->start();
}

void RunnerModel::startQuery()
{
    if (!m_manager && m_pendingQuery.isEmpty()) {
        // avoid creating a manager just so we can run nothing
        return;
    }

    createManager();
    /* DEBUG
    kWarning() << "Start query" << m_pendingQuery << "on runners:";
    Q_FOREACH(Plasma::AbstractRunner *runner, m_manager->runners()) {
        kWarning() << "-" << runner->name();
    }
    */
    m_manager->launchQuery(m_pendingQuery);
    emit queryChanged();
    m_running = true;
    emit runningChanged(true);
}

void RunnerModel::createManager()
{
    if (!m_manager) {
        m_manager = new Plasma::RunnerManager(this);
        connect(m_manager, SIGNAL(matchesChanged(QList<Plasma::QueryMatch>)),
                this, SLOT(matchesChanged(QList<Plasma::QueryMatch>)));
        connect(m_manager, SIGNAL(queryFinished()),
                this, SLOT(queryHasFinished()));

        if (!m_pendingRunnersList.isEmpty()) {
            m_manager->setAllowedRunners(m_pendingRunnersList);
            m_manager->setSingleMode(m_pendingRunnersList.count() == 1);
            m_pendingRunnersList.clear();
        }
        //connect(m_manager, SIGNAL(queryFinished()), this, SLOT(queryFinished()));
    }
}

void RunnerModel::matchesChanged(const QList<Plasma::QueryMatch> &matches)
{
    // Group matches by runner
    // We do not use a QMultiHash here because it keeps values in LIFO order, while we want FIFO.
    QHash<QString, QList<Plasma::QueryMatch> > matchesForRunner;
    Q_FOREACH(const Plasma::QueryMatch &match, matches) {
        QString runnerId = match.runner()->id();
        auto it = matchesForRunner.find(runnerId);
        if (it == matchesForRunner.end()) {
            it = matchesForRunner.insert(runnerId, QList<Plasma::QueryMatch>());
        }
        it.value().append(match);
    }

    // Assign matches to existing models. If there is no match for a model, delete it.
    for (int row = m_models.count() - 1; row >= 0; --row) {
        RunnerSubModel *subModel = m_models.at(row);
        QList<Plasma::QueryMatch> matches = matchesForRunner.take(subModel->runnerId());
        if (matches.isEmpty()) {
            beginRemoveRows(QModelIndex(), row, row);
            m_models.removeAt(row);
            delete subModel;
            endRemoveRows();
        } else {
            subModel->setMatches(matches);
        }
    }

    // At this point, matchesForRunner contains only matches for runners which
    // do not have a model yet. Create new models for them.
    if (!matchesForRunner.isEmpty()) {
        beginInsertRows(QModelIndex(), rowCount(), rowCount() + matchesForRunner.size() - 1);
        auto it = matchesForRunner.constBegin();
        auto end = matchesForRunner.constEnd();
        for (; it != end; ++it) {
            QList<Plasma::QueryMatch> matches = it.value();
            Q_ASSERT(!matches.isEmpty());
            QString name = matches.first().runner()->name();
            RunnerSubModel *subModel = new RunnerSubModel(it.key(), name, this);
            connect(subModel, SIGNAL(triggerRequested(Plasma::QueryMatch)), SLOT(trigger(Plasma::QueryMatch)));
            subModel->setMatches(matches);
            m_models.append(subModel);
        }
        endInsertRows();
    }

    m_runningChangedTimeout->start(3000);
}

void RunnerModel::queryHasFinished()
{
    m_running = false;
    emit runningChanged(false);
}

void RunnerModel::trigger(const Plasma::QueryMatch& match)
{
    m_manager->run(match);
}

//- RunnerSource ------------------------------
RunnerSource::RunnerSource(QObject *parent)
: AbstractSource(parent)
{}

QAbstractItemModel *RunnerSource::createModel(const SourceArguments &arguments)
{
    RunnerModel *model = new RunnerModel;
    QString allowed = arguments.value("whitelist");
    if (!allowed.isEmpty()) {
        QStringList runners = allowed.split(',');
        model->setAllowedRunners(runners);
    }
    return model;
};

} // namespace Homerun

#include <runnermodel.moc>
