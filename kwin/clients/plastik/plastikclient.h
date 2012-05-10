/* Plastik KWin window decoration
  Copyright (C) 2003-2005 Sandro Giessl <sandro@giessl.com>

  based on the window decoration "Web":
  Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
 */

#ifndef PLASTIKCLIENT_H
#define PLASTIKCLIENT_H

#include <kcommondecoration.h>

#include "plastik.h"

namespace KWinPlastik {


class PlastikClient : public KCommonDecoration
{
public:
    PlastikClient(KDecorationBridge* bridge, KDecorationFactory* factory);
    ~PlastikClient();

    virtual QString visibleName() const;
    virtual bool decorationBehaviour(DecorationBehaviour behaviour) const;
    virtual int layoutMetric(LayoutMetric lm, bool respectWindowState = true, const KCommonDecorationButton * = 0) const;
    virtual QRegion cornerShape(WindowCorner corner);
    virtual KCommonDecorationButton *createButton(ButtonType type);

    virtual void init();
    virtual void reset( unsigned long changed );

    virtual void paintEvent(QPaintEvent *e);
    virtual void updateCaption();

    const QPixmap &getTitleBarTile(bool active) const;

private:
    QRect captionRect() const;

    const QPixmap &captionPixmap() const;
    void clearCaptionPixmaps();

    mutable QPixmap *m_captionPixmaps[2];

    QRect m_captionRect;
    QString oldCaption;

    // settings...
    QFont s_titleFont;
};

} // KWinPlastik

#endif // PLASTIKCLIENT_H
