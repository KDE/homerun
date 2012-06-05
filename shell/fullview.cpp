/*
 * Copyright 2007 Frerich Raabe <raabe@kde.org>
 * Copyright 2007 Aaron Seigo <aseigo@kde.org>
 * Copyright 2008 Aleix Pol <aleixpol@gmail.com>
 * Copyright (C) 2012 Shaun Reich <shaun.reich@blue-systems.com>
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

#include <QDesktopWidget>
#include <QKeyEvent>

#include <kdeclarative.h>

#include <KDebug>
#include <KStandardDirs>
#include <KUrl>
#include <KWindowSystem>

#include <Plasma/FrameSvg>
#include <Plasma/WindowEffects>

FullView::FullView()
: QDeclarativeView()
, m_backgroundSvg(new Plasma::FrameSvg(this))
{
    new SalViewerAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/SalViewer", this);
    dbus.registerService("org.kde.salViewer");

    KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(engine());
    kdeclarative.initialize();
    //binds things like kconfig and icons
    kdeclarative.setupBindings();

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setResizeMode(SizeRootObjectToView);

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    viewport()->setAttribute(Qt::WA_NoSystemBackground);

    KUrl source = KGlobal::dirs()->locate("data", "plasma/plasmoids/org.kde.sal/contents/ui/main.qml");
    setSource(source);

    connect(rootObject(), SIGNAL(closeRequested()), SLOT(hide()));
    rootObject()->setProperty("embedded", true);

    setupBackground();
}

void FullView::setupBackground()
{
    if (Plasma::WindowEffects::isEffectAvailable(Plasma::WindowEffects::BlurBehind)) {
        m_backgroundSvg->setImagePath("translucent/dialogs/background");
    } else {
        m_backgroundSvg->setImagePath("dialogs/background");
    }
    qreal left, top, bottom, right;
    m_backgroundSvg->getMargins(left, top, bottom, right);
    rootObject()->setProperty("leftMargin", left);
    rootObject()->setProperty("topMargin", top);
    rootObject()->setProperty("rightMargin", right);
    rootObject()->setProperty("bottomMargin", bottom);
}

void FullView::focusOutEvent(QFocusEvent* event)
{
    event->accept();
    resetAndHide();
}

FullView::~FullView()
{
}

void FullView::toggle(int screen)
{
    if (isVisible()) {
        resetAndHide();
    } else {
        KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::KeepAbove);
        Plasma::WindowEffects::overrideShadow(winId(), true);

        QDesktopWidget w;
        const QRect rect = w.availableGeometry(screen);
        setGeometry(rect);
        show();
        KWindowSystem::forceActiveWindow(winId());
    }
}

void FullView::resetAndHide()
{
    hide();
    QMetaObject::invokeMethod(rootObject(), "reset");
}

void FullView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hide();
        event->accept();
    }

    QDeclarativeView::keyPressEvent(event);
}

void FullView::resizeEvent(QResizeEvent *event)
{
    QDeclarativeView::resizeEvent(event);
    updateGeometry();
}

void FullView::updateGeometry()
{
    m_backgroundSvg->resizeFrame(size());
    if (Plasma::WindowEffects::isEffectAvailable(Plasma::WindowEffects::BlurBehind)) {
        Plasma::WindowEffects::enableBlurBehind(winId(), true, m_backgroundSvg->mask());
    }
}

void FullView::drawBackground(QPainter *painter, const QRectF &/*rect*/)
{
    m_backgroundSvg->paintFrame(painter);
}

#include "fullview.moc"
