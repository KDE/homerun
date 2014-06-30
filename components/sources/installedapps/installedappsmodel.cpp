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
// Local
#include <changenotifier.h>
#include <pathmodel.h>
#include <actionlist.h>
#include <installedappsmodel.h>
#include <installedappsconfigurationwidget.h>
#include <sourceregistry.h>

// Qt
#include <QApplication>
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

#include <Plasma/Containment>
#include <Plasma/Corona>

namespace Homerun {

static const char *SOURCE_ID = "InstalledApps";

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

bool GroupNode::trigger(const QString &actionId, const QVariant &actionArgument)
{
    Q_UNUSED(actionId)
    Q_UNUSED(actionArgument)

    QVariantMap args;
    args.insert("entryPath", m_entryPath);
    m_model->openSourceRequested(SOURCE_ID, args);
    return false;
}

//- AppNode --------------------------------------------------------------------
AppNode::AppNode(KService::Ptr service, InstalledAppsModel *model)
: m_model(model)
, m_service(service)
{
    m_icon = service->icon();
    m_name = service->name();
    m_genericName = service->genericName();
    m_service = service;
    m_sortKey = m_name.toLower();
}

bool AppNode::trigger(const QString &actionId, const QVariant &actionArgument)
{
    Q_UNUSED(actionArgument)

    if (!actionId.isEmpty()) {
        QVariant adaptor = qApp->property("HomerunViewerAdaptor");

        if (adaptor.isValid()) {
            uint containmentId = (actionId == "addToDesktop") ? qApp->property("desktopContainmentId").toUInt()
                : qApp->property("appletContainmentId").toUInt();
            return QMetaObject::invokeMethod(adaptor.value<QObject *>(), actionId.toLocal8Bit(),
                Qt::DirectConnection, Q_ARG(uint, containmentId), Q_ARG(QString, m_service->storageId()));
        } else if (m_model->containment()) {
            Plasma::Containment *containment = static_cast<Plasma::Containment *>(m_model->containment());

            if (actionId == "addToDesktop") {
                Plasma::Containment *desktop = containment->corona()->containmentForScreen(containment->screen());

                if (desktop) {
                    if (desktop->metaObject()->indexOfSlot("addUrls(KUrl::List)") != -1) {
                        QMetaObject::invokeMethod(desktop, "addUrls",
                        Qt::DirectConnection, Q_ARG(KUrl::List, KUrl::List(m_service->entryPath())));
                    } else {
                        desktop->addApplet("icon", QVariantList() << m_service->entryPath());
                    }
                }
            } else if (actionId == "addToPanel") {
                QRectF rect(containment->geometry().width() / 3, 0, 150,
                    containment->boundingRect().height());
                containment->addApplet("icon", QVariantList() << m_service->entryPath(), rect);
            } else if (actionId == "addLauncher") {
                QObject* taskManager = 0;

                foreach(QObject* applet, containment->applets()) {
                    if (applet->metaObject()->indexOfSlot("addLauncher(QString)") != -1) {
                        taskManager = applet;
                    }
                }

                if (taskManager) {
                    QMetaObject::invokeMethod(taskManager, "addLauncher", Qt::DirectConnection,
                        Q_ARG(QString, m_service->storageId()));
                }
            }
        }
    } else {
        bool ran = KRun::run(*m_service, KUrl::List(), 0);

        if (ran) {
            emit m_model->applicationLaunched(m_service->storageId());
        }

        return ran;
    }

    return false;
}

QString AppNode::favoriteId() const
{
    return QString("app:") + m_service->storageId();
}

KService::Ptr AppNode::service() const
{
    return m_service;
}

//- InstallerNode --------------------------------------------------------------
InstallerNode::InstallerNode(KServiceGroup::Ptr group, KService::Ptr installerService)
: m_group(group)
, m_service(installerService)
{
    m_icon = m_service->icon();
    m_name = m_service->name();
}

bool InstallerNode::trigger(const QString &actionId, const QVariant &actionArgument)
{
    Q_UNUSED(actionId)
    Q_UNUSED(actionArgument)

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
InstalledAppsModel::InstalledAppsModel(const QString &entryPath, const QString &installer, QObject *parent)
: QAbstractListModel(parent)
, m_entryPath(entryPath)
, m_pathModel(new PathModel(this))
, m_installer(installer)
, m_containment(0)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::DecorationRole, "decoration");
    roles.insert(FavoriteIdRole, "favoriteId");
    roles.insert(HasActionListRole, "hasActionList");
    roles.insert(ActionListRole, "actionList");
    roles.insert(GenericNameRole, "genericName");

    setRoleNames(roles);

    refresh();
}

InstalledAppsModel::~InstalledAppsModel()
{
    qDeleteAll(m_nodeList);
}

QObject* InstalledAppsModel::containment() const
{
    return m_containment;
}

void InstalledAppsModel::setContainment(QObject* containment)
{
    m_containment = containment;
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

    AbstractNode *node = m_nodeList.at(index.row());
    if (role == Qt::DisplayRole) {
        return node->name();
    } else if (role == Qt::DecorationRole) {
        // at least show the oxygen question-mark, otherwise it looks weird blank.
        return node->icon().isEmpty() ? QLatin1String("unknown") : node->icon();
    } else if (role == FavoriteIdRole) {
        return node->favoriteId();
    } else if (role == HasActionListRole) {
        return node->type() == AbstractNode::AppNodeType;
    } else if (role == ActionListRole && node->type() == AbstractNode::AppNodeType) {
        QVariantList actionList;

        if (qApp->property("HomerunViewerAdaptor").isValid())
        {
            if (qApp->property("desktopContainmentId").toUInt() > 0
                && qApp->property("desktopContainmentMutable").toBool()) {
                actionList << ActionList::createActionItem(i18n("Add to Desktop"), "addToDesktop");
            }
            if (qApp->property("appletContainmentId").toUInt() > 0
                && qApp->property("appletContainmentMutable").toBool()) {
                actionList << ActionList::createActionItem(i18n("Add to Panel"), "addToPanel");
            }
        } else if (m_containment) {
            Plasma::Containment *containment = static_cast<Plasma::Containment *>(m_containment);
            Plasma::Containment *desktop = containment->corona()->containmentForScreen(containment->screen());

            if (desktop && desktop->immutability() == Plasma::Mutable) {
                actionList << ActionList::createActionItem(i18n("Add to Desktop"), "addToDesktop");
            }

            if (containment->immutability() == Plasma::Mutable) {
                actionList << ActionList::createActionItem(i18n("Add to Panel"), "addToPanel");
            }

            QObject* taskManager = 0;

            foreach(QObject* applet, containment->applets()) {
                if (applet->metaObject()->indexOfSlot("hasLauncher(QString)") != -1) {
                    taskManager = applet;
                }
            }

            if (taskManager) {
                AppNode* appNode = static_cast<AppNode *>(node);

                bool hasLauncher = false;

                QMetaObject::invokeMethod(taskManager, "hasLauncher", Qt::DirectConnection,
                    Q_RETURN_ARG(bool, hasLauncher), Q_ARG(QString, appNode->service()->storageId()));

                if (!hasLauncher) {
                    actionList << ActionList::createActionItem(i18n("Add as Launcher"), "addLauncher");
                }
            }
        }

        return actionList;
    } else if (role == GenericNameRole && node->type() == AbstractNode::AppNodeType) {
        return static_cast<AppNode *>(node)->genericName();
    } else if (role == CombinedNameRole && node->type() == AbstractNode::AppNodeType) {
        AppNode *appNode = static_cast<AppNode *>(node);
        return QString(appNode->name() + ' ' + appNode->genericName());
    }

    return QVariant();
}

bool InstalledAppsModel::trigger(int row, const QString &actionId, const QVariant &actionArgument)
{
    return m_nodeList.at(row)->trigger(actionId, actionArgument);
}

void InstalledAppsModel::refresh(bool reload)
{
    if (!reload) {
        emit layoutAboutToBeChanged();
        emit layoutChanged();

        return;
    }

    m_pathModel->clear();
    beginResetModel();
    qDeleteAll(m_nodeList);
    m_nodeList.clear();

    if (m_entryPath.isEmpty()) {
        loadRootEntries();
    } else {
        KServiceGroup::Ptr group = KServiceGroup::group(m_entryPath);
        loadServiceGroup(group);
        QVariantMap args;
        args.insert("entryPath", m_entryPath);
        QString label = (m_entryPath == KServiceGroup::root()->entryPath()) ? i18n("All Applications")
            : group->caption();
        m_pathModel->addPath(label, SOURCE_ID, args);
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

                bool found = false;

                foreach(const AbstractNode *node, m_nodeList) {
                    if (node->type() == AbstractNode::AppNodeType
                        && static_cast<const AppNode *>(node)->service()->storageId() == service->storageId()) {
                        found = true;
                    }
                }

                if (!found) {
                    m_nodeList << new AppNode(service, this);
                }
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
InstalledAppsSource::InstalledAppsSource(QObject *parent)
: AbstractSource(parent)
{}

QAbstractItemModel *InstalledAppsSource::createModelFromConfigGroup(const KConfigGroup &group)
{
    QString entryPath = group.readEntry("entryPath");
    return createModel(entryPath);
}

QAbstractItemModel *InstalledAppsSource::createModelFromArguments(const QVariantMap &arguments)
{
    QString entryPath = arguments.value("entryPath").toString();
    return createModel(entryPath);
}

QAbstractItemModel *InstalledAppsSource::createModel(const QString &entryPath)
{
    KConfigGroup group(config(), "PackageManagement");
    QString installer = group.readEntry("categoryInstaller");

    InstalledAppsModel *model = new InstalledAppsModel(entryPath, installer);
    ChangeNotifier *notifier = new ChangeNotifier(model);
    connect(notifier, SIGNAL(changeDetected(bool)), model, SLOT(refresh(bool)));
    return model;
}

bool InstalledAppsSource::isConfigurable() const
{
    return true;
}

SourceConfigurationWidget *InstalledAppsSource::createConfigurationWidget(const KConfigGroup &group)
{
    return new InstalledAppsConfigurationWidget(group);
}

} // namespace Homerun

#include "installedappsmodel.moc"
