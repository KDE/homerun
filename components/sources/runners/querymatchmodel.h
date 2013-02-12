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
#ifndef QUERYMATCHMODEL_H
#define QUERYMATCHMODEL_H

// Local

// KDE
#include <Plasma/QueryMatch>

// Qt
#include <QAbstractListModel>

namespace Plasma
{
    class RunnerManager;
}

namespace Homerun
{

/**
 * This model exposes results from a list of Plasma::QueryMatch
 */
class QueryMatchModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    explicit QueryMatchModel(QObject *parent = 0);

    enum {
        FavoriteIdRole = Qt::UserRole + 1,
        HasActionListRole,
        ActionListRole,
    };

    int count() const;

    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &actionArgument);

    /**
     * Sets the instance of RunnerManager which is handling the matches we show.
     * A manager *must* be defined before calling setMatches().
     */
    void setRunnerManager(Plasma::RunnerManager *manager);

public Q_SLOTS:
    void setMatches(const QList<Plasma::QueryMatch> &matches);

Q_SIGNALS:
    void countChanged();

protected:
    QList<Plasma::QueryMatch> m_matches;

private:
    Plasma::RunnerManager *m_manager = 0;
};

} // namespace

#endif /* SINGLERUNNERMODEL_H */
