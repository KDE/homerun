/***************************************************************************
 *   Copyright (C) 2012 by Shaun M. Reich <shaun.reich@kdemail.net>        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "tasks.h"

#include <Plasma/Containment>
#include <Plasma/Corona>

#include <QGraphicsView>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeView>
#include <QDeclarativeComponent>
#include <QGraphicsLinearLayout>
#include <QUrl>

#include <KDebug>

#include <Plasma/DeclarativeWidget>
#include <Plasma/Package>

#include <taskmanager/tasksmodel.h>
#include <taskmanager/groupmanager.h>

K_EXPORT_PLASMA_APPLET(tasks, Tasks)

Tasks::Tasks(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
    , m_tasksModel(0)
    , m_groupManager(0)
{
    resize(192, 128);

    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setHasConfigurationInterface(false);
}

Tasks::~Tasks()
{

}

void Tasks::init()
{

    m_groupManager = new TaskManager::GroupManager(this);

    Plasma::Containment *c = containment();

    if (c) {
        m_groupManager->setScreen(c->screen());
    }

    QGraphicsLinearLayout *lay = new QGraphicsLinearLayout(this);
    m_declarativeWidget = new Plasma::DeclarativeWidget(this);
    lay->addItem(m_declarativeWidget);

    qRegisterMetaType<TaskManager::TasksModel*>();

    m_tasksModel = new TaskManager::TasksModel(m_groupManager, this);
    Q_ASSERT(m_tasksModel);

    m_groupManager->setScreen(0);
    m_groupManager->setShowOnlyCurrentActivity(false);
    m_groupManager->setShowOnlyCurrentDesktop(false);
    m_groupManager->setShowOnlyCurrentScreen(false);
    m_groupManager->setShowOnlyMinimized(false);

    // check the model contents for debugging purposes when the data changes
    connect(m_tasksModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(checkModel()));

    Plasma::PackageStructure::Ptr structure = Plasma::PackageStructure::load("Plasma/Generic");
    m_package = new Plasma::Package(QString(), "org.kde.tasks", structure);
    m_declarativeWidget->setQmlPath(m_package->filePath("mainscript"));
    m_declarativeWidget->engine()->rootContext()->setContextProperty("tasksModel", QVariant::fromValue(static_cast<QObject *>(m_tasksModel)));
}

void Tasks::checkModel()
{
    kDebug() << "row count %%%%%%%%%%%%%%: " << m_tasksModel->rowCount();
    kDebug() << "column count %%%%%%%%%%%%%%: " << m_tasksModel->columnCount();
}

//NOTE: NEVER CALLED, JUST YET
void Tasks::configChanged()
{
    Q_ASSERT(0);
    KConfigGroup cg = config();
    bool changed = false;

    // only update these if they have actually changed, because they make the
    // group manager reload its tasks list
    const bool showOnlyCurrentDesktop = cg.readEntry("showOnlyCurrentDesktop", false);
    if (showOnlyCurrentDesktop != m_groupManager->showOnlyCurrentDesktop()) {
        m_groupManager->setShowOnlyCurrentDesktop(showOnlyCurrentDesktop);
        changed = true;
    }

    const bool showOnlyCurrentActivity = cg.readEntry("showOnlyCurrentActivity", true);
    if (showOnlyCurrentActivity != m_groupManager->showOnlyCurrentActivity()) {
        m_groupManager->setShowOnlyCurrentActivity(showOnlyCurrentActivity);
        changed = true;
    }

    const bool showOnlyCurrentScreen = cg.readEntry("showOnlyCurrentScreen", false);
    if (showOnlyCurrentScreen != m_groupManager->showOnlyCurrentScreen()) {
        m_groupManager->setShowOnlyCurrentScreen(showOnlyCurrentScreen);
        changed = true;
    }

    const bool showOnlyMinimized = cg.readEntry("showOnlyMinimized", false);
    if (showOnlyMinimized != m_groupManager->showOnlyMinimized()) {
        m_groupManager->setShowOnlyMinimized(showOnlyMinimized);
        changed = true;
    }

    TaskManager::GroupManager::TaskGroupingStrategy groupingStrategy =
    static_cast<TaskManager::GroupManager::TaskGroupingStrategy>(
        cg.readEntry("groupingStrategy",
                     static_cast<int>(TaskManager::GroupManager::ProgramGrouping))
    );
    if (groupingStrategy != m_groupManager->groupingStrategy()) {
        m_groupManager->setGroupingStrategy(groupingStrategy);
        changed = true;
    }
//FIXME: everything basically ;)
    const bool onlyGroupWhenFull = cg.readEntry("groupWhenFull", true);
    if (onlyGroupWhenFull != m_groupManager->onlyGroupWhenFull()) {
//        adjustGroupingStrategy();
        m_groupManager->setOnlyGroupWhenFull(onlyGroupWhenFull);
        changed = true;
    }

    TaskManager::GroupManager::TaskSortingStrategy sortingStrategy =
    static_cast<TaskManager::GroupManager::TaskSortingStrategy>(
        cg.readEntry("sortingStrategy",
                     static_cast<int>(TaskManager::GroupManager::AlphaSorting))
    );

    if (sortingStrategy != m_groupManager->sortingStrategy()) {
        m_groupManager->setSortingStrategy(sortingStrategy);
        changed = true;
    }

//    const int maxRows = cg.readEntry("maxRows", 2);
//    if (maxRows != m_rootGroupItem->maxRows()) {
//        m_rootGroupItem->setMaxRows(maxRows);
//        changed = true;
//    }
//
//    const bool forceRows = cg.readEntry("forceRows", false);
//    if (forceRows != m_rootGroupItem->forceRows()) {
//        m_rootGroupItem->setForceRows(forceRows);
//        changed = true;
//    }
//
//    const bool showTooltip = cg.readEntry("showTooltip", true);
//    if (showTooltip != m_showTooltip) {
//        m_showTooltip = showTooltip;
//        changed = true;
//    }
//
//    const bool highlightWindows = cg.readEntry("highlightWindows", false);
//    if (highlightWindows != m_highlightWindows) {
//        m_highlightWindows = highlightWindows;
//        changed = true;
//    }
//
    m_groupManager->readLauncherConfig();

    if (changed) {
//        emit settingsChanged();
        update();
    }
}

#include "tasks.moc"
