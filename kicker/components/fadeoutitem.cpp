/*
 *   Copyright 2013 Eike Hein <hein@kde.org>
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "fadeoutitem.h"
#include "fadeouteffect.h"

#include <QGraphicsOpacityEffect>

#include <KDebug>

FadeOutItem::FadeOutItem(QDeclarativeItem *parent) : QDeclarativeItem(parent)
{
    setFlag(QDeclarativeItem::ItemHasNoContents, false);
}

FadeOutItem::~FadeOutItem()
{
}

int FadeOutItem::covered() const
{
    if (graphicsEffect()) {
        return static_cast<FadeOutEffect *>(graphicsEffect())->covered();
    }

    return 0;
}

void FadeOutItem::setCovered(int covered)
{
    if (graphicsEffect() && covered == 0) {
        setGraphicsEffect(0);
    }

    if (covered > 0) {
        if (!graphicsEffect()) {
            setGraphicsEffect(new FadeOutEffect());
        }

        static_cast<FadeOutEffect *>(graphicsEffect())->setCovered(covered);
    }
}

#include "fadeoutitem.moc"
