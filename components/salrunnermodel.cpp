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
#include <salrunnermodel.h>

// Local

// KDE
#include <KDebug>
#include <Plasma/AbstractRunner>
#include <Plasma/RunnerManager>

// Qt
#include <QStandardItemModel>
#include <QTimer>

SalRunnerSubModel::SalRunnerSubModel(const QString &runnerId, const QString &name, QObject *parent)
: QAbstractListModel(parent)
, m_runnerId(runnerId)
, m_name(name)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "label");
    roles.insert(Qt::DecorationRole, "icon");
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

int SalRunnerSubModel::count() const
{
    return m_matches.count();
}

int SalRunnerSubModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_matches.count();
}

QVariant SalRunnerSubModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.parent().isValid() ||
        index.column() > 0 || index.row() < 0 || index.row() >= m_matches.count()) {
        // index requested must be valid, but we have no child items!
        //kDebug() << "invalid index requested";
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return m_matches.at(index.row()).text();
    } else if (role == Qt::DecorationRole) {
        return m_matches.at(index.row()).icon();
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

void SalRunnerSubModel::setMatches(const QList<Plasma::QueryMatch> &matches)
{
    //kDebug() << "got matches:" << matches.count();
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

//--------------------------------------------------------------------

SalRunnerModel::SalRunnerModel(QObject *parent)
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

SalRunnerModel::~SalRunnerModel()
{
}

int SalRunnerModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_models.count();
}

QVariant SalRunnerModel::data(const QModelIndex &index, int role) const
{
    kWarning() << index << role;
    if (index.parent().isValid()) {
        return QVariant();
    }

    SalRunnerSubModel *model = m_models.value(index.row());
    if (!model) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return model->name();
    } else {
        return QVariant();
    }
}

QObject *SalRunnerModel::modelForRow(int row) const
{
    return m_models.value(row);
}

QStringList SalRunnerModel::arguments() const
{
    return m_manager ? m_manager->allowedRunners() : m_pendingRunnersList;
}

void SalRunnerModel::setArguments(const QStringList& args)
{
    QSet<QString> argsSet = args.toSet();
    if (arguments().toSet() == argsSet) {
        return;
    }
    if (m_manager) {
        m_manager->setAllowedRunners(args);

        //automagically enter single runner mode if there's only 1 allowed runner
        m_manager->setSingleMode(args.count() == 1);
    } else {
        m_pendingRunnersList = args;
    }
    emit argumentsChanged();
}

QString SalRunnerModel::currentQuery() const
{
    return m_manager ? m_manager->query() : QString();
}

void SalRunnerModel::scheduleQuery(const QString &query)
{
    m_pendingQuery = query;
    m_startQueryTimer->start();
}

void SalRunnerModel::startQuery()
{
    if (!m_manager && m_pendingQuery.isEmpty()) {
        // avoid creating a manager just so we can run nothing
        return;
    }

    //kDebug() << "booooooo yah!!!!!!!!!!!!!" << query;
    createManager();

//    if (m_pendingQuery != m_manager->query()) {
        //kDebug() << "running query" << query;
        m_manager->launchQuery(m_pendingQuery);
        emit queryChanged();
        m_running = true;
        emit runningChanged(true);
 //   }
}

void SalRunnerModel::createManager()
{
    kWarning();
    if (!m_manager) {
        kWarning() << "Creating manager";
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

void SalRunnerModel::matchesChanged(const QList<Plasma::QueryMatch> &matches)
{
    kWarning() << "matches.count" << matches.count();
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

    // Assign matches to existing models. If there is no match for a model, the model has to be deleted.
    // Keep track of rows to remove.
    QList<int> rowsToRemove;
    for (int row = m_models.count() - 1; row >= 0; --row) {
        SalRunnerSubModel *subModel = m_models.at(row);
        QList<Plasma::QueryMatch> matches = matchesForRunner.take(subModel->runnerId());
        if (matches.isEmpty()) {
            rowsToRemove.append(row);
        } else {
            subModel->setMatches(matches);
        }
    }

    // Now matchesForRunner contains only matches for runners which do not have a model yet
    // Create new models for them
    if (!matchesForRunner.isEmpty()) {
        beginInsertRows(QModelIndex(), rowCount(), rowCount() + matchesForRunner.size());
        auto it = matchesForRunner.constBegin();
        auto end = matchesForRunner.constEnd();
        for (; it != end; ++it) {
            QList<Plasma::QueryMatch> matches = it.value();
            Q_ASSERT(!matches.isEmpty());
            QString name = matches.first().runner()->name();
            SalRunnerSubModel *subModel = new SalRunnerSubModel(it.key(), name, this);
            subModel->setMatches(matches);
            m_models.append(subModel);
        }
        endInsertRows();
    }

    // Remove rows of empty models. Rows are kept in decreasing orders, so when the Nth one is dropped, the N+1 one is still valid
    Q_FOREACH(int row, rowsToRemove) {
        beginRemoveRows(QModelIndex(), row, row);
        delete m_models.takeAt(row);
        endRemoveRows();
    }
    
    m_runningChangedTimeout->start(3000);
}

void SalRunnerModel::queryHasFinished()
{
    m_running = false;
    emit runningChanged(false);
}

#include <salrunnermodel.moc>
