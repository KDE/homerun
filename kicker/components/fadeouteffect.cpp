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

#include "fadeouteffect.h"

#include <QLinearGradient>
#include <QPainter>

FadeOutEffect::FadeOutEffect(QObject *parent) : QGraphicsEffect(parent)
, m_covered(0)
{
}

FadeOutEffect::~FadeOutEffect()
{
}

int FadeOutEffect::covered() const
{
    return m_covered;
}

void FadeOutEffect::setCovered(int covered)
{
    m_covered = covered;

    update();
}

void FadeOutEffect::draw(QPainter* painter)
{
    QPixmap sourceItem = sourcePixmap(Qt::LogicalCoordinates);

    QPixmap pixmap(sourceItem.size());
    pixmap.fill(Qt::transparent);

    QLinearGradient gradient(0, qMax(0, pixmap.size().height() - m_covered - 8),
        0, pixmap.size().height() - m_covered);
    gradient.setColorAt(0, "black");
    gradient.setColorAt(1, Qt::transparent);

    QPainter itemPainter(&pixmap);
    itemPainter.drawPixmap(0, 0, sourceItem);
    itemPainter.setBrush(QBrush(gradient));
    itemPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    itemPainter.drawRect(QRect(0, 0, pixmap.size().width(), pixmap.size().height()));

    painter->drawPixmap(0, 0, pixmap);
}


#include "fadeouteffect.moc"
