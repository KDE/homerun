/*
 *  Copyright (C) 2003 by Unai Garro <ugarro@users.sourceforge.net>
 *  Copyright (C) 2004 by Enrico Ros <rosenric@dei.unipd.it>
 *  Copyright (C) 2004 by Stephan Kulow <coolo@kde.org>
 *  Copyright (C) 2004 by Oswald Buddenhagen <ossi@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "kdmrect.h"
#include "kdmthemer.h"

#include <QPainter>

KdmRect::KdmRect(QObject *parent, const QDomNode &node)
    : KdmItem(parent, node)
{
    itemType = "rect";
    if (!isVisible())
        return;

    // Set default values for rect (note: strings are already Null)
    rect.active.present = false;
    rect.prelight.present = false;

    // Read RECT TAGS
    QDomNodeList childList = node.childNodes();
    for (int nod = 0; nod < childList.count(); nod++) {
        QDomNode child = childList.item(nod);
        QDomElement el = child.toElement();
        QString tagName = el.tagName();

        if (tagName == "normal") {
            parseColor(el, rect.normal.color);
        } else if (tagName == "active") {
            rect.active.present = true;
            parseColor(el, rect.active.color);
        } else if (tagName == "prelight") {
            rect.prelight.present = true;
            parseColor(el, rect.prelight.color);
        }
    }
}

void
KdmRect::drawContents(QPainter *p, const QRect &r)
{
    // choose the correct rect class
    RectStruct::RectClass *rClass = &rect.normal;
    if (state == Sactive && rect.active.present)
        rClass = &rect.active;
    if (state == Sprelight && rect.prelight.present)
        rClass = &rect.prelight;

    if (!rClass->color.isValid())
        return;

    p->fillRect(r, QBrush(rClass->color));
}

void
KdmRect::statusChanged(bool descend)
{
    KdmItem::statusChanged(descend);
    if (!rect.active.present && !rect.prelight.present)
        return;
    if ((state == Sprelight && !rect.prelight.present) ||
        (state == Sactive && !rect.active.present))
        return;
    needUpdate();
}

#include "kdmrect.moc"
