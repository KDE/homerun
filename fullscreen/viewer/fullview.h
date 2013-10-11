/*
 * Copyright 2007 Aaron Seigo <aseigo@kde.org
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

#ifndef FULLVIEW_H
#define FULLVIEW_H

#include <QDeclarativeView>

namespace Plasma {
class FrameSvg;
}

class FullView : public QDeclarativeView
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.homerunViewer")

public:
    FullView();
    ~FullView();

    bool init(QString *error);

    void setConfigFileName(const QString &name);

public Q_SLOTS:
    /**
     * If the view is not visible, show it in @p screen. If @p screen is -1
     * the used screen will be the one that contains the mouse
     * 
     * If it is visible, then hide the view.
     */
    void toggle(int screen, uint appletContainmentId, bool appletContainmentMutable,
        int desktopContainmentId, bool desktopContainmentMutable);
    void updateGeometry();

protected:
    virtual void focusOutEvent(QFocusEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
    virtual void drawBackground(QPainter *painter, const QRectF&rect);

private Q_SLOTS:
    void logFocusedItem();

private:
    void setupBackground();
    void resetAndHide();

    Plasma::FrameSvg *m_backgroundSvg;

    QGraphicsItem *m_lastFocusedItem;

    bool m_plainWindow;
};

#endif

