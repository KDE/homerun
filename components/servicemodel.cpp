/*
    Copyright 2011 Aaron Seigo <aseigo@kde.org>
    Copyright (C) 2012 Shaun Reich <shaun.reich@blue-systems.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// Local
#include <pathmodel.h>
#include <servicemodel.h>
#include <sourcearguments.h>

// Qt
#include <QIcon>
#include <QAction>
#include <QTimer>

// KDE
#include <KDebug>
#include <kmacroexpander.h>
#include <KRun>
#include <KService>
#include <KServiceTypeTrader>
#include <KSycocaEntry>

#include <Plasma/RunnerManager>

//- AbstractNode ---------------------------------------------------------------
AbstractNode::~AbstractNode()
{
}

bool AbstractNode::lessThan(AbstractNode *n1, AbstractNode *n2)
{
    Q_ASSERT(n1);
    Q_ASSERT(n2);
    return n1->m_sortKey < n2->m_sortKey;
}

//- GroupNode ------------------------------------------------------------------
GroupNode::GroupNode(KServiceGroup::Ptr group, ServiceModel *model)
: m_model(model)
{
    m_icon = KIcon(group->icon());
    m_name = group->caption();
    m_entryPath = group->entryPath();
    m_sortKey = m_name.toLower();
}

bool GroupNode::trigger()
{
    QString source = "ServiceModel:entryPath=" % SourceArguments::escapeValue(m_entryPath);
    QMetaObject::invokeMethod(m_model, "openSourceRequested", Q_ARG(QString, source));
    return false;
}

//- AppNode --------------------------------------------------------------------
AppNode::AppNode(KService::Ptr service)
: m_service(service)
{
    m_icon = KIcon(service->icon());
    m_name = service->name();
    m_service = service;
    m_sortKey = m_name.toLower();
}

bool AppNode::trigger()
{
    return KRun::run(*m_service, KUrl::List(), 0);
}

QString AppNode::favoriteId() const
{
    return QString("app:") + m_service->storageId();
}

//- InstallerNode --------------------------------------------------------------
InstallerNode::InstallerNode(KServiceGroup::Ptr group, KService::Ptr installerService)
: m_group(group)
, m_service(installerService)
{
    m_icon = KIcon(m_service->icon());
    m_name = m_service->name();
}

bool InstallerNode::trigger()
{
    QHash<QString, QString> map;
    QString category = m_group->entryPath();
    if (category.endsWith('/')) {
        category.truncate(category.length() - 1);
    }
    map.insert("category", category);

    QString command = KMacroExpander::expandMacros(m_service->exec(), map, '@');
    return KRun::run(command, KUrl::List(), 0, m_service->name(), m_service->icon());
}

//- ServiceModel ------------------------------------------------------------
ServiceModel::ServiceModel (QObject *parent)
: QAbstractListModel(parent)
, m_pathModel(new PathModel(this))
{
    load(QString());

    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "label");
    roles.insert(Qt::DecorationRole, "icon");
    roles.insert(FavoriteIdRole, "favoriteId");

    setRoleNames(roles);
}

ServiceModel::~ServiceModel()
{
    qDeleteAll(m_nodeList);
}

int ServiceModel::rowCount(const QModelIndex& index) const
{
    return index.isValid() ? 0 : m_nodeList.count();
}

int ServiceModel::count() const
{
    return m_nodeList.count();
}

QVariant ServiceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_nodeList.count()) {
        return QVariant();
    }

    const AbstractNode *node = m_nodeList.at(index.row());
    if (role == Qt::DisplayRole) {
        return node->name();
    } else if (role == Qt::DecorationRole) {
        return node->icon();
    } else if (role == FavoriteIdRole) {
        return node->favoriteId();
    }

    return QVariant();
}

bool ServiceModel::trigger(int row)
{
    return m_nodeList.at(row)->trigger();
}

void ServiceModel::load(const QString &entryPath)
{
    m_pathModel->clear();
    beginResetModel();
    qDeleteAll(m_nodeList);
    m_nodeList.clear();

    if (entryPath.isEmpty()) {
        loadRootEntries();
    } else {
        KServiceGroup::Ptr group = KServiceGroup::group(entryPath);
        loadServiceGroup(group);
        QString source = "ServiceModel:entryPath=" % SourceArguments::escapeValue(entryPath);
        m_pathModel->addPath(group->caption(), source);
    }

    endResetModel();

    emit countChanged();
}

void ServiceModel::setInstaller(const QString& installer)
{
    if (installer == m_installer) {
        return;
    }
    m_installer = installer;
    installerChanged(installer);
}

QString ServiceModel::installer() const
{
    return m_installer;
}

void ServiceModel::loadRootEntries()
{
    KServiceGroup::Ptr group = KServiceGroup::root();
    KServiceGroup::List list = group->entries(false /* sorted: set to false as it does not seem to work */);

    for( KServiceGroup::List::ConstIterator it = list.constBegin(); it != list.constEnd(); it++) {
        const KSycocaEntry::Ptr p = (*it);

        if (p->isType(KST_KServiceGroup)) {
            KServiceGroup::Ptr subGroup = KServiceGroup::Ptr::staticCast(p);

            if (!subGroup->noDisplay() && subGroup->childCount() > 0) {
                m_nodeList << new GroupNode(subGroup, this);
            }
        }
    }
    qSort(m_nodeList.begin(), m_nodeList.end(), AbstractNode::lessThan);
}

void ServiceModel::loadServiceGroup(KServiceGroup::Ptr group)
{
    doLoadServiceGroup(group);

    qSort(m_nodeList.begin(), m_nodeList.end(), AbstractNode::lessThan);

    if (!m_installer.isEmpty()) {
        KService::Ptr service = KService::serviceByDesktopName(m_installer);
        if (service) {
            m_nodeList << new InstallerNode(group, service);
        } else {
            kWarning() << "Could not find service for" << m_installer;
        }
    }
}

void ServiceModel::doLoadServiceGroup(KServiceGroup::Ptr group)
{
    /* This method is separate from loadServiceGroup so that
     * - only one installer node is added at the end
     * - sorting is done only once
     */
    if (!group || !group->isValid()) {
        return;
    }

    KServiceGroup::List list = group->entries(false /* see above */);

    for( KServiceGroup::List::ConstIterator it = list.constBegin();
        it != list.constEnd(); it++) {
        const KSycocaEntry::Ptr p = (*it);

        if (p->isType(KST_KService)) {
            const KService::Ptr service = KService::Ptr::staticCast(p);

            if (!service->noDisplay()) {
                QString genericName = service->genericName();
                if (genericName.isNull()) {
                    genericName = service->comment();
                }
                m_nodeList << new AppNode(service);
            }

        } else if (p->isType(KST_KServiceGroup)) {
            const KServiceGroup::Ptr subGroup = KServiceGroup::Ptr::staticCast(p);

            if (!subGroup->noDisplay() && subGroup->childCount() > 0) {
                doLoadServiceGroup(subGroup);
            }
        }
    }
}

QString ServiceModel::arguments() const
{
    return m_arguments;
}

void ServiceModel::setArguments(const QString& arguments)
{
    if (m_arguments == arguments) {
        return;
    }

    SourceArguments::Hash args = SourceArguments::parse(arguments);
    load(args.value("entryPath"));

    m_arguments = arguments;
    argumentsChanged(m_arguments);
}

PathModel *ServiceModel::pathModel() const
{
    return m_pathModel;
}

#include "servicemodel.moc"