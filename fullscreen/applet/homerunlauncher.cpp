/***************************************************************************
 *   Copyright (C) 2012 by Shaun Reich <shaun.reich@blue-systems.com>      *
 *   Copyright (C) 2013 by Eike Hein <hein@kde.org>                        *
 *   Copyright 2007 Robert Knight <robertknight@gmail.com>                 *
 *   Copyright 2007 Kevin Ottens <ervin@kde.org>                           *
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

// Qt
#include <QAction>
#include <QGraphicsLinearLayout>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusServiceWatcher>

// KDE
#include <KAuthorized>
#include <KConfigDialog>
#include <KProcess>
#include <KRun>
#include <KStandardDirs>
#include <KShortcut>
#include <KWindowSystem>
#include <Plasma/IconWidget>
#include <Plasma/Containment>
#include <Plasma/Corona>

// Local
#include <configkeys.h>
#include <homerunlauncher.h>

HomerunLauncher::HomerunLauncher(QObject * parent, const QVariantList & args)
    : Plasma::Applet(parent, args),
      m_icon(0),
      m_serviceRegistered(QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.homerunViewer")),
      m_toggleWhenRegistered(false)
{
    setHasConfigurationInterface(true);
    KGlobal::locale()->insertCatalog("plasma_applet_org.kde.homerun");
}

void HomerunLauncher::init()
{
    if (KService::serviceByStorageId("kde4-kmenuedit.desktop") && KAuthorized::authorize("action/menuedit")) {
        QAction* menueditor = new QAction(i18n("Edit Applications..."), this);
        actions.append(menueditor);
        connect(menueditor, SIGNAL(triggered(bool)), this, SLOT(startMenuEditor()));
    }

    m_serviceWatcher = new QDBusServiceWatcher("org.kde.homerunViewer", QDBusConnection::sessionBus(),
        QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(m_serviceWatcher, SIGNAL(serviceRegistered(QString)), this, SLOT(viewerServiceRegistered()));
    connect(m_serviceWatcher, SIGNAL(serviceUnregistered(QString)), this, SLOT(viewerServiceUnregistered()));

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_icon = new Plasma::IconWidget(this);

    connect(m_icon, SIGNAL(clicked()), SLOT(toggle()));
    connect(this, SIGNAL(activate()), SLOT(toggle()));

    layout->addItem(m_icon);

    configChanged();

    if (!m_serviceRegistered) {
        kDebug() << "Service not registered, launching homerunviewer";
        startViewer();
    }
}

QList<QAction *> HomerunLauncher::contextualActions()
{
    return actions;
}

void HomerunLauncher::startMenuEditor()
{
    KProcess::execute("kmenuedit");
}

void HomerunLauncher::startViewer()
{
    KRun::runCommand("homerunviewer", 0);
}

void HomerunLauncher::toggle()
{
    if (!m_serviceRegistered) {
        m_toggleWhenRegistered = true;
        startViewer();
        return;
    }

    uint appletContainmentId = 0;
    bool appletContainmentMutable = false;
    uint desktopContainmentId = 0;
    bool desktopContainmentMutable = false;

    if (containment()) {
        if (containment()->containmentType() == Plasma::Containment::PanelContainment
            || containment()->containmentType() == Plasma::Containment::CustomPanelContainment) {
            appletContainmentId = containment()->id();
        }
        appletContainmentMutable = containment()->immutability() == Plasma::Mutable;

        Plasma::Containment *desktop = containment()->corona()->containmentForScreen(containment()->screen());

        if (desktop) {
            desktopContainmentId = desktop->id();
            desktopContainmentMutable = desktop->immutability() == Plasma::Mutable;
        }
    }

    QDBusConnection bus = QDBusConnection::sessionBus();
    QDBusInterface interface("org.kde.homerunViewer", "/HomerunViewer", "org.kde.homerunViewer", bus);
    interface.asyncCall("toggle", containment()->screen(), appletContainmentId, appletContainmentMutable,
        desktopContainmentId, desktopContainmentMutable);
}

void HomerunLauncher::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget;
    m_generalUi.setupUi(widget);
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    parent->addPage(widget, i18n("General"), "homerun");

    m_generalUi.iconButton->setIcon(m_icon->icon());

    connect(m_generalUi.iconButton, SIGNAL(iconChanged(QString)), parent, SLOT(settingsModified()));
}

void HomerunLauncher::configChanged()
{
    m_icon->setIcon(config().readEntry(CFG_LAUNCHER_ICON_KEY, CFG_LAUNCHER_ICON_DEFAULT));
}

void HomerunLauncher::configAccepted()
{
    KConfigGroup cg = config();

    cg.writeEntry(CFG_LAUNCHER_ICON_KEY, m_generalUi.iconButton->icon());

    emit configNeedsSaving();
}

void HomerunLauncher::viewerServiceRegistered()
{
    m_serviceRegistered = true;

    if (m_toggleWhenRegistered) {
        toggle();
        m_toggleWhenRegistered = false;
    }

    QDBusConnection::sessionBus().connect("org.kde.homerunViewer", "/HomerunViewer",
        "org.kde.homerunViewer", "addToDesktop", this, SLOT(addToDesktop(uint,QString)));
    QDBusConnection::sessionBus().connect("org.kde.homerunViewer", "/HomerunViewer",
        "org.kde.homerunViewer", "addToPanel", this, SLOT(addToPanel(uint,QString)));
}

void HomerunLauncher::viewerServiceUnregistered()
{
    m_serviceRegistered = false;

    QDBusConnection::sessionBus().disconnect("org.kde.homerunViewer", "/HomerunViewer",
        "org.kde.homerunViewer", "addToDesktop", this, SLOT(addToDesktop(uint,QString)));
    QDBusConnection::sessionBus().disconnect("org.kde.homerunViewer", "/HomerunViewer",
        "org.kde.homerunViewer", "addToPanel", this, SLOT(addToPanel(uint,QString)));
}

void HomerunLauncher::addToDesktop(uint containmentId, const QString &storageId)
{
    Plasma::Containment *desktop = containment()->corona()->containmentForScreen(containment()->screen());
    KService::Ptr service = KService::serviceByStorageId(storageId);

    if (!desktop || !service || desktop->id() != containmentId) {
        return;
    }

    if (desktop->metaObject()->indexOfSlot("addUrls(KUrl::List)") != -1) {
        QMetaObject::invokeMethod(desktop, "addUrls",
        Qt::DirectConnection, Q_ARG(KUrl::List, KUrl::List(service->entryPath())));
    } else {
        desktop->addApplet("icon", QVariantList() << service->entryPath());
    }
}

void HomerunLauncher::addToPanel(uint containmentId, const QString &storageId)
{
    KService::Ptr service = KService::serviceByStorageId(storageId);

    if (service || containment()->id() == containmentId) {
        // move it to the middle of the panel
        QRectF rect(containment()->geometry().width() / 3, 0, 150, containment()->boundingRect().height());
        containment()->addApplet("icon", QVariantList() << service->entryPath(), rect);
    }

}

#include "homerunlauncher.moc"
