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
#ifndef PAGEMODEL_H
#define PAGEMODEL_H

#include <QAbstractListModel>

#include <KSharedConfig>

class Page;

/**
 * A PageModel loads the definition of SAL pages from a configuration file
 */
class PageModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum {
        IconNameRole = Qt::UserRole + 1,
        SourcesRole,
    };

    PageModel(QObject *parent = 0);
    ~PageModel();

    void setConfig(const KSharedConfig::Ptr &);

    int rowCount(const QModelIndex &parent = QModelIndex()) const; // reimp
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const; // reimp

private:
    KSharedConfig::Ptr m_config;
    QList<Page*> m_pageList;
};

#endif /* PAGEMODEL_H */
