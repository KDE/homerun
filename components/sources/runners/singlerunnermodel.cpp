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
SingleRunnerModel::SingleRunnerModel(const KConfigGroup &configGroup, QObject *parent)
: QueryMatchModel(parent)
, m_configGroup(configGroup)
, m_manager(0)
{
}

void SingleRunnerModel::init(const QString& runnerId)
{
    if (!m_manager) {
        // RunnerManager must have its own config group to store instance-specific config
        // (we don't want the manager from this RunnerModel to overwrite the config from another RunnerModel manager)
        m_manager = new Plasma::RunnerManager(m_configGroup, this);
        connect(m_manager, SIGNAL(matchesChanged(QList<Plasma::QueryMatch>)), SLOT(setMatches(QList<Plasma::QueryMatch>)));
        setRunnerManager(m_manager);
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
    if (!m_manager->singleModeRunner()->defaultSyntax()) {
        kWarning() << "Runner" << runnerId << "advertise itself as a single mode runner but does not provide a default syntax!";
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
    if (!m_manager) {
        return;
    }
    if (m_query == value) {
        return;
    }
    m_query = value;
    QString term = prepareSearchTerm(m_query);
    m_manager->launchQuery(term, m_manager->singleModeRunnerId());
    queryChanged(m_query);
}

QString SingleRunnerModel::name() const
{
    if (m_manager) {
        return m_manager->singleModeRunner()->name();
    } else {
        return i18n("Single Runner");
    }
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

//- SingleRunnerSource ------------------------
SingleRunnerSource::SingleRunnerSource(const QString &runnerId, QObject *parent)
: AbstractSource(parent)
, m_runnerId(runnerId)
{}

QAbstractItemModel *SingleRunnerSource::createModelFromConfigGroup(const KConfigGroup &group)
{
    SingleRunnerModel *model = new SingleRunnerModel(group);
    model->init(m_runnerId);
    return model;
};

} // namespace Homerun

#include <singlerunnermodel.moc>
