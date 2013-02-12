/*
Copyright 2013 Aurélien Gâteau <agateau@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
// Self
#include <singlerunnermodel.h>

// Homerun
#include <abstractsource.h>
#include <actionlist.h>

// KDE
#include <KDebug>
#include <KLocale>
#include <KPluginFactory>
#include <Plasma/RunnerManager>

// Qt
#include <QIcon>

static const char *RUNNER_KEY = "runner";

SingleRunnerModel::SingleRunnerModel()
: m_manager(0)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::DecorationRole, "decoration");
    roles.insert(HasActionListRole, "hasActionList");
    roles.insert(ActionListRole, "actionList");
    setRoleNames(roles);
}

void SingleRunnerModel::init(const QString& runnerId)
{
    if (!m_manager) {
        m_manager = new Plasma::RunnerManager(this);
        connect(m_manager, SIGNAL(matchesChanged(QList<Plasma::QueryMatch>)), SLOT(slotMatchesChanged(QList<Plasma::QueryMatch>)));
    }
    m_manager->setAllowedRunners(QStringList() << runnerId);
    m_manager->setSingleModeRunnerId(runnerId);
    m_manager->setSingleMode(true);
    if (!m_manager->singleModeRunner()) {
        kWarning() << "Failed to load runner" << runnerId << "as a single mode runner";
        delete m_manager;
        m_manager = 0;
        return;
    }
    m_query = "$bypass-same-value-check";
    setQuery(QString());
}

QString SingleRunnerModel::query() const
{
    return m_query;
}

void SingleRunnerModel::setQuery(const QString &value)
{
    if (m_query == value) {
        return;
    }
    m_query = value;
    QString term = prepareSearchTerm(m_query);
    m_manager->launchQuery(term, m_manager->singleModeRunnerId());
    queryChanged(m_query);
}

bool SingleRunnerModel::trigger(int row, const QString &actionId, const QVariant &/*actionArgument*/)
{
    if (actionId.isEmpty()) {
        Plasma::QueryMatch match = m_matches.at(row);
        if (match.isEnabled()) {
            m_manager->run(match);
            return true;
        } else {
            return false;
        }
    }
    return false;
}

QString SingleRunnerModel::name() const
{
    if (m_manager) {
        return m_manager->singleModeRunner()->name();
    } else {
        return i18n("Single Runner");
    }
}

int SingleRunnerModel::count() const
{
    return m_matches.count();
}

int SingleRunnerModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_matches.count();
}

QVariant SingleRunnerModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= m_matches.count()) {
        return QVariant();
    }
    Plasma::QueryMatch match = m_matches.at(index.row());
    if (role == Qt::DisplayRole) {
        return match.text();
    } else if (role == Qt::DecorationRole) {
        return match.icon();
    } else if (role == HasActionListRole) {
        return false;
    }
    return QVariant();
}

QString SingleRunnerModel::prepareSearchTerm(const QString &term)
{
    const char *placeHolder = ":q:";
    Q_ASSERT(m_manager);
    Plasma::RunnerSyntax *syntax = m_manager->singleModeRunner()->defaultSyntax();
    QStringList queries = syntax->exampleQueries();
    Q_ASSERT(!queries.isEmpty());
    QString query = queries.first();
    if (query.contains(placeHolder)) {
        return query.replace(placeHolder, term);
    } else {
        return query + ' ' + term;
    }
}

void SingleRunnerModel::slotMatchesChanged(const QList< Plasma::QueryMatch > &matches)
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

//- SingleRunnerSource ------------------------
SingleRunnerSource::SingleRunnerSource(QObject *parent)
: Homerun::AbstractSource(parent)
{}

QAbstractItemModel *SingleRunnerSource::createModelFromConfigGroup(const KConfigGroup &group)
{
    SingleRunnerModel *model = new SingleRunnerModel;
    QString runner = group.readEntry(RUNNER_KEY, QString());
    model->init(runner);
    return model;
};

bool SingleRunnerSource::isConfigurable() const
{
    return false;
}

Homerun::SourceConfigurationWidget *SingleRunnerSource::createConfigurationWidget(const KConfigGroup &group)
{
    return 0;//new RunnerConfigurationWidget(group);
}

#include <singlerunnermodel.moc>
