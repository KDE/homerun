/*
 * Copyright 2012 Aurélien Gâteau <agateau@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
// Self
#include <haloeffect.h>

// Local

// KDE
#include <Plasma/PaintUtils>

// Qt
#include <QPainter>

HaloEffect::HaloEffect(QObject *parent)
: QGraphicsEffect(parent)
{
}

HaloEffect::~HaloEffect()
{
}

void HaloEffect::draw(QPainter *painter)
{
    updateRects();
    Q_FOREACH(const QRectF &rect, m_rects) {
        Plasma::PaintUtils::drawHalo(painter, rect);
    }
    drawSource(painter);
}

void HaloEffect::updateRects()
{
    if (!m_rects.isEmpty()) {
        return;
    }

    const QImage img = sourcePixmap().toImage();
    const int width = img.width();
    const int height = img.height();

    if (width < 4 || height < 4) {
        return;
    }

    const int blockHeight = 16;
    const int blockCount = height / blockHeight + 1;
    QVector<int> offsets(blockCount, width);
    for (int y = 0; y < height; ++y) {
        QRgb *start = (QRgb *)(img.scanLine(y));
        QRgb *end = start + width;
        QRgb *ptr = start;
        for (; ptr != end; ++ptr) {
            if (qAlpha(*ptr) > 0) {
                int idx = y / blockHeight;
                offsets[idx] = qMin(int(ptr - start), offsets[idx]);
                break;
            }
        }
    }
    m_rects.reserve(blockCount);
    const QPointF topLeft = boundingRect().topLeft();
    for (int block = 0; block < blockCount; ++block) {
        int offset = offsets.at(block);
        m_rects << QRectF(
            topLeft.x() + offset,
            topLeft.y() + block * blockHeight,
            width - 2 * offset, // Take advantage of the fact that text is centered
            blockHeight);
    }
}

void HaloEffect::sourceChanged(QGraphicsEffect::ChangeFlags /*flags*/)
{
    m_rects.clear();
}

#include <haloeffect.moc>
