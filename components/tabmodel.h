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
#ifndef TABMODEL_H
#define TABMODEL_H

#include <QAbstractListModel>

#include <KSharedConfig>

namespace Homerun {
class AbstractSourceRegistry;
}
class Tab;

/**
 * A TabModel loads the definition of Homerun tabs from a configuration file
 */
class TabModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString configFileName READ configFileName WRITE setConfigFileName NOTIFY configFileNameChanged)
    Q_PROPERTY(Homerun::AbstractSourceRegistry *sourceRegistry READ sourceRegistry WRITE setSourceRegistry NOTIFY sourceRegistryChanged)

public:
    enum {
        SourceModelRole = Qt::UserRole + 1,
    };

    TabModel(QObject *parent = 0);
    ~TabModel();

    void setConfig(const KSharedConfig::Ptr &);

    QString configFileName() const;
    void setConfigFileName(const QString &name);

    Homerun::AbstractSourceRegistry *sourceRegistry() const;
    void setSourceRegistry(Homerun::AbstractSourceRegistry *registry);

    int rowCount(const QModelIndex &parent = QModelIndex()) const; // reimp
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const; // reimp

    Q_INVOKABLE void setDataForRow(int row, const QByteArray &role, const QVariant &value);

    Q_INVOKABLE void appendRow();

    Q_INVOKABLE void removeRow(int row);

    Q_INVOKABLE void moveRow(int from, int to);

    /**
     * Reset configuration to default setup
     */
    Q_INVOKABLE void resetConfig();

Q_SIGNALS:
    void configFileNameChanged(const QString &);
    void sourceRegistryChanged();

private:
    KSharedConfig::Ptr m_config;
    QList<Tab*> m_tabList;
    Homerun::AbstractSourceRegistry *m_sourceRegistry;

    QStringList tabGroupList() const;

    void writeGeneralTabsEntry();
};

#endif /* TABMODEL_H */
