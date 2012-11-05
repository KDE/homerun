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
#ifndef SOURCEMODEL_H
#define SOURCEMODEL_H

// Local

// Qt
#include <QAbstractListModel>

// KDE
#include <KConfigGroup>

namespace Homerun
{
class AbstractSourceRegistry;
}

class SourceModelItem;

/**
 *
 */
class SourceModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit SourceModel(Homerun::AbstractSourceRegistry *registry, const KConfigGroup &tabGroup, QObject *parent);
    ~SourceModel();

    enum {
        SourceIdRole = Qt::UserRole + 1,
        ModelRole,
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const; // reimp
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const; // reimp

    void reload();

    Q_INVOKABLE void appendSource(const QString &sourceId);

private:
    Homerun::AbstractSourceRegistry *m_sourceRegistry;
    KConfigGroup m_tabGroup;
    QList<SourceModelItem *> m_list;

    void writeSourcesEntry();
};

#endif /* SOURCEMODEL_H */
