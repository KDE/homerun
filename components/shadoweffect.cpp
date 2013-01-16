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
#include <shadoweffect.h>

// Local

// KDE
#include <Plasma/PaintUtils>

// Qt
#include <QPainter>
#include <QPaintEngine>

ShadowEffect::ShadowEffect(QObject *parent)
: QGraphicsEffect(parent)
, m_xOffset(0)
, m_yOffset(1)
, m_blurRadius(3)
, m_color(Qt::black)
{
}

ShadowEffect::~ShadowEffect()
{
}

qreal ShadowEffect::xOffset() const
{
    return m_xOffset;
}

qreal ShadowEffect::yOffset() const
{
    return m_yOffset;
}

qreal ShadowEffect::blurRadius() const
{
    return m_blurRadius;
}

QColor ShadowEffect::color() const
{
    return m_color;
}

#define SETTER(prop) \
    if (prop == value) { \
        return; \
    } \
    prop = value;

#define SETTER_REAL(prop) \
    if (qFuzzyCompare(prop, value)) { \
        return; \
    } \
    prop = value;

void ShadowEffect::setXOffset(qreal value)
{
    SETTER_REAL(m_xOffset);
    m_shadow = QImage();
    updateBoundingRect();
    xOffsetChanged(m_xOffset);
}

void ShadowEffect::setYOffset(qreal value)
{
    SETTER_REAL(m_yOffset);
    m_shadow = QImage();
    updateBoundingRect();
    yOffsetChanged(m_yOffset);
}

void ShadowEffect::setBlurRadius(qreal value)
{
    SETTER_REAL(m_blurRadius);
    m_shadow = QImage();
    updateBoundingRect();
    blurRadiusChanged(m_blurRadius);
}

void ShadowEffect::setColor(const QColor &value)
{
    SETTER(m_color);
    m_shadow = QImage();
    update();
    colorChanged(m_color);
}

QRectF ShadowEffect::boundingRectFor(const QRectF &rect) const
{
    return rect.united(
        rect.translated(m_xOffset, m_yOffset)
        .adjusted(-m_blurRadius, -m_blurRadius, m_blurRadius, m_blurRadius)
        );
}

QImage ShadowEffect::generateShadow(const QPixmap &px) const
{
    if (px.isNull()) {
        return QImage();
    }

    // Generate shadow in tmp
    QImage tmp(px.size(), QImage::Format_ARGB32_Premultiplied);
    tmp.fill(0);
    QPainter tmpPainter(&tmp);
    tmpPainter.setCompositionMode(QPainter::CompositionMode_Source);
    tmpPainter.drawPixmap(m_xOffset, m_yOffset, px);
    tmpPainter.end();
    Plasma::PaintUtils::shadowBlur(tmp, m_blurRadius, m_color);
    return tmp;
}

void ShadowEffect::draw(QPainter *painter)
{
    if (m_blurRadius <= 0 && qFuzzyIsNull(m_xOffset) && qFuzzyIsNull(m_yOffset)) {
        drawSource(painter);
        return;
    }

    PixmapPadMode mode = PadToEffectiveBoundingRect;
    if (painter->paintEngine()->type() == QPaintEngine::OpenGL2) {
        mode = NoPad;
    }

    // Draw pixmap in device coordinates to avoid pixmap scaling.
    QPoint offset;
    const QPixmap pixmap = sourcePixmap(Qt::DeviceCoordinates, &offset, mode);
    if (pixmap.isNull()) {
        return;
    }

    QTransform restoreTransform = painter->worldTransform();
    painter->setWorldTransform(QTransform());
    if (m_shadow.isNull()) {
        m_shadow = generateShadow(pixmap);
    }
    // Draw shadow (draw it twice to darken it)
    painter->drawImage(offset, m_shadow);
    painter->drawImage(offset, m_shadow);
    // Draw the actual pixmap
    painter->drawPixmap(offset, pixmap);
    painter->setWorldTransform(restoreTransform);
}

void ShadowEffect::sourceChanged(ChangeFlags flags)
{
    switch (flags) {
    case SourceAttached:
    case SourceBoundingRectChanged:
    case SourceInvalidated:
        m_shadow = QImage();
        break;
    case SourceDetached:
        break;
    }
}

#include <shadoweffect.moc>
