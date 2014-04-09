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
// Self
#include <runnermodel.h>

// Local
#include <runnerconfigurationwidget.h>

// KDE
#include <KDebug>
#include <KPluginInfo>
#include <Plasma/AbstractRunner>
#include <Plasma/RunnerManager>

// Qt
#include <QStandardItemModel>
#include <QTimer>

static const char *WHITELIST_KEY = "whitelist";

namespace Homerun {

RunnerSubModel::RunnerSubModel(const QString &runnerId, const QString &name, Plasma::RunnerManager *manager, QObject *parent)
: QueryMatchModel(parent)
, m_runnerId(runnerId)
, m_name(name)
{
    setRunnerManager(manager);
}

//--------------------------------------------------------------------
RunnerModel::RunnerModel(const KConfigGroup &configGroup, QObject *parent)
: QAbstractListModel(parent)
, m_configGroup(configGroup)
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

    QStringList lst = m_configGroup.readEntry(WHITELIST_KEY, QStringList());
    setAllowedRunners(lst);
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
    m_pendingRunnersList = list;
    if (m_manager) {
        loadRunners();
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
    if (m_pendingQuery.isEmpty()) {
        clear();
    }

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
        // RunnerManager must have its own config group to store instance-specific config
        // (we don't want the manager from this RunnerModel to overwrite the config from another RunnerModel manager)
        m_manager = new Plasma::RunnerManager(m_configGroup, this);
        connect(m_manager, SIGNAL(matchesChanged(QList<Plasma::QueryMatch>)),
                this, SLOT(matchesChanged(QList<Plasma::QueryMatch>)));
        connect(m_manager, SIGNAL(queryFinished()),
                this, SLOT(queryHasFinished()));

        if (!m_pendingRunnersList.isEmpty()) {
            loadRunners();
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
            RunnerSubModel *subModel = new RunnerSubModel(it.key(), name, m_manager, this);
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

void RunnerModel::clear()
{
    if (m_models.isEmpty()) {
        return;
    }
    beginResetModel();
    qDeleteAll(m_models);
    m_models.clear();
    endResetModel();
}

void RunnerModel::loadRunners()
{
    Q_ASSERT(m_manager);

    // FIXME: SC 4.13 replaced Nepomuk with Baloo for desktop search. Homerun's
    // default configs reference Nepomuk's "nepomuksearch" runner. The following
    // is a runtime approach to rewriting this to "baloosearch" when found. This
    // keeps things working on <4.13 while enabling 4.13+ compatibility. It can
    // be dropped once we depend on a SC version guaranteed to have Baloo around.
    KPluginInfo::List runners = m_manager->listRunnerInfo();

    foreach(const KPluginInfo &runner, runners) {
        if (runner.pluginName() == "baloosearch") {
            m_pendingRunnersList.replaceInStrings("nepomuksearch", "baloosearch");
            m_pendingRunnersList.removeDuplicates();

            // Update config.
            QStringList whiteList = m_configGroup.readEntry("whitelist", QStringList());

            if (!whiteList.isEmpty()) {
                whiteList.replaceInStrings("nepomuksearch", "baloosearch");
                whiteList.removeDuplicates();

                m_configGroup.writeEntry("whitelist", whiteList);

                if (m_configGroup.config()) {
                    m_configGroup.config()->sync();
                }
            }

            break;
        }
    }
    // FIXME: </Baloo hack>

    if (m_pendingRunnersList.count() > 0) {
        KPluginInfo::List list = Plasma::RunnerManager::listRunnerInfo();
        Q_FOREACH(const KPluginInfo &info, list) {
            if (m_pendingRunnersList.contains(info.pluginName())) {
                m_manager->loadRunner(info.service());
            }
        }
    }
    m_manager->setSingleMode(m_pendingRunnersList.count() == 1);
    m_pendingRunnersList.clear();
}

Plasma::RunnerManager *RunnerModel::manager() const
{
    return m_manager;
}

//- RunnerSource ------------------------------
RunnerSource::RunnerSource(QObject *parent)
: AbstractSource(parent)
{}

QAbstractItemModel *RunnerSource::createModelFromConfigGroup(const KConfigGroup &group)
{
    return new RunnerModel(group);
};

bool RunnerSource::isConfigurable() const
{
    return true;
}

SourceConfigurationWidget *RunnerSource::createConfigurationWidget(const KConfigGroup &group)
{
    return new RunnerConfigurationWidget(group);
}

} // namespace Homerun

#include <runnermodel.moc>
