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
#include <installedappsmodel.h>
#include <sourceid.h>
#include <sourceregistry.h>

// Qt
#include <QIcon>
#include <QAction>
#include <QTimer>

// KDE
#include <KDebug>
#include <KLocale>
#include <kmacroexpander.h>
#include <KRun>
#include <KService>
#include <KServiceTypeTrader>
#include <KSycocaEntry>

#include <Plasma/RunnerManager>

namespace Homerun {

static QString sourceString(const QString &entryPath)
{
    SourceId sourceId;
    sourceId.setName("InstalledApps");
    sourceId.arguments().add("entryPath", entryPath);
    return sourceId.toString();
}

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
GroupNode::GroupNode(KServiceGroup::Ptr group, InstalledAppsModel *model)
: m_model(model)
{
    m_icon = group->icon();
    m_name = group->caption();
    m_entryPath = group->entryPath();
    m_sortKey = m_name.toLower();
}

bool GroupNode::trigger()
{
    QString source = sourceString(m_entryPath);
    QMetaObject::invokeMethod(m_model, "openSourceRequested", Q_ARG(QString, source));
    return false;
}

//- AppNode --------------------------------------------------------------------
AppNode::AppNode(KService::Ptr service)
: m_service(service)
{
    m_icon = service->icon();
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
    m_icon = m_service->icon();
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

//- InstalledAppsModel ------------------------------------------------------------
InstalledAppsModel::InstalledAppsModel (QObject *parent)
: QAbstractListModel(parent)
, m_pathModel(new PathModel(this))
{
    load(QString());

    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::DecorationRole, "decoration");
    roles.insert(FavoriteIdRole, "favoriteId");

    setRoleNames(roles);
}

InstalledAppsModel::~InstalledAppsModel()
{
    qDeleteAll(m_nodeList);
}

int InstalledAppsModel::rowCount(const QModelIndex& index) const
{
    return index.isValid() ? 0 : m_nodeList.count();
}

int InstalledAppsModel::count() const
{
    return m_nodeList.count();
}

QVariant InstalledAppsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_nodeList.count()) {
        return QVariant();
    }

    const AbstractNode *node = m_nodeList.at(index.row());
    if (role == Qt::DisplayRole) {
        return node->name();
    } else if (role == Qt::DecorationRole) {
        // at least show the oxygen question-mark, otherwise it looks weird blank.
        return node->icon().isEmpty() ? QLatin1String("unknown") : node->icon();
    } else if (role == FavoriteIdRole) {
        return node->favoriteId();
    }

    return QVariant();
}

bool InstalledAppsModel::trigger(int row)
{
    return m_nodeList.at(row)->trigger();
}

void InstalledAppsModel::load(const QString &entryPath)
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
        QString source = sourceString(entryPath);
        m_pathModel->addPath(group->caption(), source);
    }

    endResetModel();

    emit countChanged();
}

void InstalledAppsModel::loadRootEntries()
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

void InstalledAppsModel::loadServiceGroup(KServiceGroup::Ptr group)
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

void InstalledAppsModel::doLoadServiceGroup(KServiceGroup::Ptr group)
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

PathModel *InstalledAppsModel::pathModel() const
{
    return m_pathModel;
}

QString InstalledAppsModel::name() const
{
    if (m_pathModel->count() > 0) {
        QModelIndex index = m_pathModel->index(m_pathModel->count() - 1, 0);
        return index.data().toString();
    } else {
        return i18n("Applications");
    }
}

//- InstalledAppsSource ---------------------------------------------
InstalledAppsSource::InstalledAppsSource(SourceRegistry *registry)
: AbstractSource(registry)
{}

QAbstractItemModel *InstalledAppsSource::createModel(const SourceArguments &arguments)
{
    InstalledAppsModel *model = new InstalledAppsModel;

    KConfigGroup group(registry()->config(), "PackageManagement");
    model->m_installer = group.readEntry("categoryInstaller");

    model->load(arguments.value("entryPath"));

    return model;
}

} // namespace Homerun

#include "installedappsmodel.moc"
