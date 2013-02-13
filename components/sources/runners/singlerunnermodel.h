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
#ifndef SINGLERUNNERMODEL_H
#define SINGLERUNNERMODEL_H

// Local
#include <abstractsource.h>
#include <querymatchmodel.h>

// KDE
#include <Plasma/RunnerManager>

namespace Homerun
{

/**
 * This model exposes results from a runner with single-runner mode
 */
class SingleRunnerModel : public QueryMatchModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
public:
    explicit SingleRunnerModel(Plasma::RunnerManager *manager, QObject * parent = 0);

    QString name() const;

    void launchQuery(const QString &query);

private:
    KConfigGroup m_configGroup;
    Plasma::RunnerManager *m_manager;

    QString prepareSearchTerm(const QString &term);
};

/**
 * This model is the same as SingleRunnerModel, except it exposes a "query" property.
 * It is used when the Runner default syntax has a placeholder.
 */
class SingleQueriableRunnerModel : public SingleRunnerModel
{
    Q_OBJECT
    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
public:
    explicit SingleQueriableRunnerModel(Plasma::RunnerManager *manager, QObject * parent = 0);

    QString query() const;
    void setQuery(const QString &query);

Q_SIGNALS:
    void queryChanged(const QString &);

private:
    QString m_query;
};

class SingleRunnerSource : public AbstractSource
{
public:
    SingleRunnerSource(const QString &runnerId, QObject *parent);
    QAbstractItemModel *createModelFromConfigGroup(const KConfigGroup &group) override;

private:
    QString m_runnerId;
};

} // namespace Homerun

#endif /* SINGLERUNNERMODEL_H */
