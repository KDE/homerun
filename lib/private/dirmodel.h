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
    };

    Q_INVOKABLE bool trigger(int row);

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
    QAbstractItemModel *createModel(const KConfigGroup &group);
    QAbstractItemModel *createModelForArguments(const QVariantMap &args);
    bool isConfigurable() const;
    SourceConfigurationWidget *createConfigurationWidget(const KConfigGroup &group);

private:
    QAbstractItemModel *createModel(const KUrl &rootUrl, const QString &rootName, const KUrl &url);
};

} // namespace Homerun

#endif /* DIRMODEL_H */
