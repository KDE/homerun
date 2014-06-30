/*
Copyright 2012 Aurélien Gâteau <agateau@kde.org>
Copyright 2013 Eike Hein <hein@kde.org>

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
#ifndef INSTALLEDAPPSMODEL_H
#define INSTALLEDAPPSMODEL_H

// Local
#include <abstractsource.h>

// Qt
#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QStringList>

// KDE
#include <KIcon>
#include <KService>
#include <KServiceGroup>
#include <KUrl>

namespace Homerun {

class PathModel;

class InstalledAppsModel;
class InstalledAppsSource;

class AbstractNode
{
public:
    enum NodeType { AppNodeType, GroupNodeType, InstallerNodeType };

    virtual ~AbstractNode();

    virtual NodeType type() const = 0;

    virtual bool trigger(const QString &actionId = QString(), const QVariant &actionArgument = QVariant()) = 0;
    virtual QString favoriteId() const { return QString(); }

    QString icon() const { return m_icon; }
    QString name() const { return m_name; }
    QString genericName() const { return m_genericName; }

    static bool lessThan(AbstractNode *n1, AbstractNode *n2);

protected:
    QString m_sortKey;
    QString m_icon;
    QString m_name;
    QString m_genericName;
};

class GroupNode : public AbstractNode
{
public:
    GroupNode(KServiceGroup::Ptr group, InstalledAppsModel *model);

    NodeType type() const { return GroupNodeType; }

    bool trigger(const QString &actionId = QString(), const QVariant &actionArgument = QVariant()); // reimp

private:
    InstalledAppsModel *m_model;
    QString m_entryPath;
};

class AppNode : public AbstractNode
{
public:
    AppNode(KService::Ptr service, InstalledAppsModel *model);

    NodeType type() const { return AppNodeType; }

    bool trigger(const QString &actionId = QString(), const QVariant &actionArgument = QVariant()); // reimp;
    QString favoriteId() const; // reimp

    KService::Ptr service() const;

private:
    InstalledAppsModel *m_model;
    KService::Ptr m_service;
};

class InstallerNode : public AbstractNode
{
public:
    InstallerNode(KServiceGroup::Ptr group, KService::Ptr installerService);

    NodeType type() const { return InstallerNodeType; }

    bool trigger(const QString &actionId = QString(), const QVariant &actionArgument = QVariant()); // reimp;

private:
    InstalledAppsModel *m_model;
    KServiceGroup::Ptr m_group;
    KService::Ptr m_service;
};

class InstalledAppsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QObject* pathModel READ pathModel CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QObject* containment READ containment WRITE setContainment)

public:
    enum Roles {
        FavoriteIdRole = Qt::UserRole + 1,
        HasActionListRole,
        ActionListRole,
        GenericNameRole,
        CombinedNameRole
    };

    InstalledAppsModel(const QString &entryPath, const QString &installer, QObject *parent = 0);
    ~InstalledAppsModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int count() const;
    QVariant data(const QModelIndex &, int) const;

    PathModel *pathModel() const;

    Q_INVOKABLE bool trigger(int row, const QString &actionId = QString(), const QVariant &actionArgument = QVariant());

    QObject *containment() const;
    void setContainment(QObject *containment);

    QString name() const;

Q_SIGNALS:
    void countChanged();
    void openSourceRequested(const QString &sourceId, const QVariantMap &args);
    void applicationLaunched(const QString& storageId);

public Q_SLOTS:
    void refresh(bool reload = true);

private:
    void loadRootEntries();
    void loadServiceGroup(KServiceGroup::Ptr group);
    void doLoadServiceGroup(KServiceGroup::Ptr group);

    QString m_entryPath;
    PathModel *m_pathModel;
    QList<AbstractNode *> m_nodeList;
    QString m_installer;
    QString m_arguments;

    QObject *m_containment;

    friend class GroupNode;
    friend class AppNode;
};

class InstalledAppsSource : public AbstractSource
{
public:
    InstalledAppsSource(QObject *parent);
    QAbstractItemModel *createModelFromConfigGroup(const KConfigGroup &group); // reimp
    QAbstractItemModel *createModelFromArguments(const QVariantMap &arguments); // reimp

    bool isConfigurable() const; // reimp
    SourceConfigurationWidget *createConfigurationWidget(const KConfigGroup &group); // reimp

private:
    QAbstractItemModel *createModel(const QString &entryPath);
};

} // namespace Homerun

#endif
