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

namespace Homerun
{

//- SingleRunnerModel -------------------------
SingleRunnerModel::SingleRunnerModel(Plasma::RunnerManager *manager, QObject *parent)
: QueryMatchModel(parent)
, m_manager(manager)
{
    setRunnerManager(manager);
    connect(m_manager, SIGNAL(matchesChanged(QList<Plasma::QueryMatch>)), SLOT(setMatches(QList<Plasma::QueryMatch>)));
    launchQuery(QString());
}

void SingleRunnerModel::launchQuery(const QString &query)
{
    QString term = prepareSearchTerm(query);
    m_manager->launchQuery(term, m_manager->singleModeRunnerId());
}

QString SingleRunnerModel::name() const
{
    return m_manager->singleModeRunner()->name();
}

QString SingleRunnerModel::prepareSearchTerm(const QString &term)
{
    const char *placeHolder = ":q:";
    Plasma::RunnerSyntax *syntax = m_manager->singleModeRunner()->defaultSyntax();
    QStringList queries = syntax->exampleQueries();
    Q_ASSERT(!queries.isEmpty());
    QString query = queries.first();
    return query.replace(placeHolder, term);
}

//- SingleQueriableRunnerModel ----------------
SingleQueriableRunnerModel::SingleQueriableRunnerModel(Plasma::RunnerManager *manager, QObject *parent)
: SingleRunnerModel(manager, parent)
{
}

QString SingleQueriableRunnerModel::query() const
{
    return m_query;
}

void SingleQueriableRunnerModel::setQuery(const QString &value)
{
    if (m_query == value) {
        return;
    }
    m_query = value;
    launchQuery(m_query);
    queryChanged(m_query);
}

//- SingleRunnerSource ------------------------
SingleRunnerSource::SingleRunnerSource(const QString &runnerId, QObject *parent)
: AbstractSource(parent)
, m_runnerId(runnerId)
{}

QAbstractItemModel *SingleRunnerSource::createModelFromConfigGroup(const KConfigGroup &group_)
{
    KConfigGroup group(group_);
    // RunnerManager must have its own config group to store instance-specific config
    // (we don't want the manager from this RunnerModel to overwrite the config from another RunnerModel manager)
    Plasma::RunnerManager *manager = new Plasma::RunnerManager(group, this);
    manager->setAllowedRunners(QStringList() << m_runnerId);
    manager->setSingleModeRunnerId(m_runnerId);
    manager->setSingleMode(true);
    if (!manager->singleModeRunner()) {
        kWarning() << "Failed to load runner" << m_runnerId << "as a single mode runner";
        delete manager;
        return 0;
    }

    Plasma::RunnerSyntax *syntax = manager->singleModeRunner()->defaultSyntax();
    if (!syntax) {
        kWarning() << "Runner" << m_runnerId << "advertises itself as a single mode runner but does not provide a default syntax!";
        delete manager;
        return 0;
    }
    QStringList queries = syntax->exampleQueries();
    Q_ASSERT(!queries.isEmpty());
    QString query = queries.first();

    if (query.contains(":q:")) {
        return new SingleQueriableRunnerModel(manager);
    } else {
        return new SingleRunnerModel(manager);
    }
};

} // namespace Homerun

#include <singlerunnermodel.moc>
