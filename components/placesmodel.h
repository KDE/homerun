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
#ifndef PLACESMODEL_H
#define PLACESMODEL_H

#include <QSortFilterProxyModel>

#include <KDirSortFilterProxyModel>
#include <KUrl>

class KDirLister;
class KFileItem;
class KFilePlacesModel;

/**
 * Internal
 */
class ProxyDirModel : public KDirSortFilterProxyModel
{
public:
    explicit ProxyDirModel(QObject *parent = 0);

    KFileItem itemForIndex(const QModelIndex &index) const;

    KDirLister *dirLister() const;
};

/**
 * Adapts KFilePlacesModel to make it SAL friendly
 */
class PlacesModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)

public:
    enum {
        FavoriteActionRole = Qt::UserRole,
    };

    PlacesModel(QObject *parent = 0);
    Q_INVOKABLE bool trigger(int row);
    Q_INVOKABLE void triggerFavoriteActionAt(int row);

    int count() const;

    QString path() const;
    void setPath(const QString &path);

    QString filter() const;
    void setFilter(const QString &filter);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const; // reimp

    void addPlace(const QString &text, const KUrl &url);

Q_SIGNALS:
    void countChanged();
    void filterChanged();
    void pathChanged(const QString &);

private:
    KFilePlacesModel *m_placesModel;
    ProxyDirModel *m_proxyDirModel;
    KUrl m_rootUrl;
    QString m_rootName;
    QString m_filter;

    void switchToPlacesModel();
    void switchToDirModel();
    void openDirUrl(const KUrl &url);
};

#endif /* PLACESMODEL_H */
