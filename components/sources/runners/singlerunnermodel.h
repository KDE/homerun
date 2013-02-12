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

// KDE
#include <QAbstractListModel>
#include <Plasma/RunnerManager>

/**
 * This model exposes results from a runner with single-runner mode
 */
class SingleRunnerModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
public:
    SingleRunnerModel();

    enum {
        HasActionListRole = Qt::UserRole + 1,
        ActionListRole,
    };

    Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &actionArgument);

    QString name() const;

    int count() const;

    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void init(const QString &runnerId);

    QString query() const;
    void setQuery(const QString &query);

Q_SIGNALS:
    void countChanged();
    void queryChanged(const QString &);

private Q_SLOTS:
    void slotMatchesChanged(const QList<Plasma::QueryMatch> &matches);

private:
    Plasma::RunnerManager *m_manager;

    QString m_query;
    QList<Plasma::QueryMatch> m_matches;

    QString prepareSearchTerm(const QString &term);
};

class SingleRunnerSource : public Homerun::AbstractSource
{
public:
    SingleRunnerSource(QObject *parent);
    QAbstractItemModel *createModelFromConfigGroup(const KConfigGroup &group);
    Homerun::SourceConfigurationWidget *createConfigurationWidget(const KConfigGroup &group);
    bool isConfigurable() const;
};

#endif /* SINGLERUNNERMODEL_H */
