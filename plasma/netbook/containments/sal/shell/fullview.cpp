/*
 * Copyright 2007 Frerich Raabe <raabe@kde.org>
 * Copyright 2007 Aaron Seigo <aseigo@kde.org>
 * Copyright 2008 Aleix Pol <aleixpol@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "fullview.h"

#include "salvieweradaptor.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QResizeEvent>
#include <QTimer>
#include <QDesktopWidget>
#include <QPushButton>
#include <QGraphicsView>

#include <KCmdLineArgs>
#include <KIconLoader>
#include <KWindowSystem>

#include <Plasma/AccessManager>
#include <Plasma/AccessAppletJob>
#include <Plasma/Containment>
#include <Plasma/ContainmentActions>
#include <Plasma/Package>
#include <Plasma/Wallpaper>
#include <Plasma/WindowEffects>
#include <Plasma/PushButton>
using namespace Plasma;

FullView::FullView(const QString &ff, const QString &loc, bool persistent, QWidget *parent)
    : QGraphicsView(),
      m_formfactor(Plasma::Planar),
      m_location(Plasma::Floating),
      m_containment(0),
      m_corona(0),
      m_applet(0),
      m_appletShotTimer(0),
      m_persistentConfig(persistent),
      m_closeButton(0)
{
    new SalViewerAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/SalViewer", this);
    dbus.registerService("org.kde.salViewer");

    m_corona = new Plasma::Corona(this);
    setScene(m_corona);

    m_applet = Plasma::Applet::load("org.kde.sal");

    if (!m_applet) {
        kDebug() << "failed to load";
        return;
    }

    m_containment = m_corona->addContainment("null");
    m_containment->addApplet(m_applet, QPointF(-1, -1), false);
    m_containment->resize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    m_applet->setPos(0, 0);
    m_applet->setFlag(QGraphicsItem::ItemIsMovable, false);
    setSceneRect(m_applet->sceneBoundingRect());
    setWindowTitle(m_applet->name());
    setWindowIcon(SmallIcon(m_applet->icon()));
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    setAttribute(Qt::WA_NoSystemBackground);
    viewport()->setAttribute(Qt::WA_NoSystemBackground);
    Plasma::WindowEffects::overrideShadow(winId(), true);

//    m_applet->addAction(QString("remove"), KStandardAction::quit(this, SLOT(hide()), m_applet));
    // enforce the applet being our size
    connect(m_applet, SIGNAL(geometryChanged()), this, SLOT(updateGeometry()));
    updateGeometry();
}

void FullView::focusInEvent(QFocusEvent* event)
{
    kDebug() << "FOCUS IN";
    QWidget::focusInEvent(event);
}

void FullView::focusOutEvent(QFocusEvent* event)
{
    kDebug() << "FOCUS OUT!!";
    hide();
}

FullView::~FullView()
{
    m_containment->destroy(false);
    kDebug() << "DTOR HIT";
//    storeCurrentApplet();
    delete m_closeButton;
}

void FullView::showEvent(QShowEvent *event)
{
    
}

void FullView::showPopup(int screen)
{
    kDebug() << "POPUP REQUESTED";
    KWindowSystem::setState( winId(), NET::SkipTaskbar | NET::SkipPager | NET::KeepAbove );
    KWindowSystem::forceActiveWindow(winId());

    QDesktopWidget w;
    const QRect rect = w.availableGeometry(screen);

    kDebug() << "Rect, w: " << rect.width() << " h: " << rect.height();

    setGeometry(rect);
    show();
}

void FullView::keyPressEvent(QKeyEvent *event)
{
    kDebug() << "KEYPRESS";
    if (event->key() == Qt::Key_Escape) {
        hide();
        event->accept();
    }
    QGraphicsView::keyPressEvent(event);
}

void FullView::closeEvent(QCloseEvent *event)
{
    kDebug() << "CLOSE EVENT";
}

void FullView::setContainment(Plasma::Containment *c)
{
    if (m_containment) {
        disconnect(m_containment, 0, this, 0);
    }

    m_containment = c;
    updateGeometry();
}

void FullView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    updateGeometry();
//    emit geometryChanged();
}

void FullView::updateGeometry()
{
    if (!m_containment) {
        return;
    }

    //kDebug() << "New applet geometry is" << m_applet->geometry();

    if (m_applet) {
        if (m_applet->size().toSize() != size()) {
            m_applet->resize(size());
        }

        setSceneRect(m_applet->sceneBoundingRect());
    }

    if ((windowFlags() & Qt::FramelessWindowHint) &&
        m_applet->backgroundHints() != Plasma::Applet::NoBackground) {

        // TODO: Use the background's mask for blur
        QRegion mask;
    mask += QRect(QPoint(), size());

    Plasma::WindowEffects::enableBlurBehind(winId(), true, mask);
        }
}

#include "fullview.moc"
