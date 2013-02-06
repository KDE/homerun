/*
Copyright 2012 Aurélien Gâteau <agateau@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef DIRMODEL_H
#define DIRMODEL_H

// Local
#include <abstractsource.h>

// Qt

// KDE
#include <KDirSortFilterProxyModel>
#include <KUrl>

class KDirLister;

namespace Homerun {

class PathModel;

/**
 * Internal
 */
class DirModel : public KDirSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString name READ name CONSTANT)

    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(QObject *pathModel READ pathModel CONSTANT)
    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
public:
    explicit DirModel(QObject *parent = 0);

    void init(const KUrl &rootUrl, const QString &rootName, const KUrl &url);

    enum {
        FavoriteIdRole = Qt::UserRole + 1,
        HasActionListRole,
        ActionListRole,
    };

    Q_INVOKABLE bool trigger(int row, const QString &actionId = QString(), const QVariant &actionArgument = QVariant());

    KDirLister *dirLister() const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const; // reimp

    PathModel *pathModel() const;

    int count() const;

    QString name() const;

    bool running() const;

    QString query() const;

    void setQuery(const QString &query);

    static QVariantMap sourceArguments(const KUrl &rootUrl, const QString &rootName, const KUrl &url);

Q_SIGNALS:
    void countChanged();
    void runningChanged(bool);
    void openSourceRequested(const QString &sourceId, const QVariantMap &sourceArguments);
    void queryChanged(const QString &);

private Q_SLOTS:
    void emitRunningChanged();

private:
    PathModel *m_pathModel;
    KUrl m_rootUrl;
    QString m_rootName;

    void initPathModel(const KUrl &url);
};

class DirSource : public AbstractSource
{
public:
    DirSource(QObject *parent);
    QAbstractItemModel *createModelFromConfigGroup(const KConfigGroup &group);
    QAbstractItemModel *createModelFromArguments(const QVariantMap &args);
    bool isConfigurable() const;
    SourceConfigurationWidget *createConfigurationWidget(const KConfigGroup &group);

private:
    QAbstractItemModel *createModel(const KUrl &rootUrl, const QString &rootName, const KUrl &url);
};

} // namespace Homerun

#endif /* DIRMODEL_H */
