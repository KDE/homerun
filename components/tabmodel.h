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
#ifndef TABMODEL_H
#define TABMODEL_H

#include <QAbstractListModel>

#include <KSharedConfig>

class Tab;

/**
 * A TabModel loads the definition of Homerun tabs from a configuration file
 */
class TabModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString configFileName READ configFileName WRITE setConfigFileName NOTIFY configFileNameChanged)

public:
    enum {
        SourcesRole = Qt::UserRole + 1,
        SearchPlaceholderRole,
    };

    TabModel(QObject *parent = 0);
    ~TabModel();

    void setConfig(const KSharedConfig::Ptr &);

    QString configFileName() const;
    void setConfigFileName(const QString &name);

    int rowCount(const QModelIndex &parent = QModelIndex()) const; // reimp
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const; // reimp

    Q_INVOKABLE void setSourcesForRow(int row, const QVariant &sources);

Q_SIGNALS:
    void configFileNameChanged(const QString &);

private:
    KSharedConfig::Ptr m_config;
    QList<Tab*> m_tabList;
};

#endif /* TABMODEL_H */
