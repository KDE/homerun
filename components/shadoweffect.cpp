/*
 * Copyright 2013 Aurélien Gâteau <agateau@kde.org>
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
#include <KDebug>
#include <Plasma/PaintUtils>

// Qt
#include <QPainter>
#include <QPaintEngine>

ShadowEffect::ShadowEffect(QObject *parent)
: QGraphicsEffect(parent)
, m_xOffset(0)
, m_yOffset(1)
, m_blurRadius(3)
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

void ShadowEffect::setXOffset(qreal value)
{
    if (qFuzzyCompare(m_xOffset, value)) {
        return;
    }
    m_xOffset = value;
    m_shadow = QImage();
    updateBoundingRect();
    xOffsetChanged(m_xOffset);
}

void ShadowEffect::setYOffset(qreal value)
{
    if (qFuzzyCompare(m_yOffset, value)) {
        return;
    }
    m_yOffset = value;
    m_shadow = QImage();
    updateBoundingRect();
    yOffsetChanged(m_yOffset);
}

void ShadowEffect::setBlurRadius(qreal value)
{
    if (qFuzzyCompare(m_blurRadius, value)) {
        return;
    }
    m_blurRadius = value;
    m_shadow = QImage();
    updateBoundingRect();
    blurRadiusChanged(m_blurRadius);
}

void ShadowEffect::setColor(const QColor &value)
{
    if (m_color == value) {
        return;
    }
    m_color = value;
    m_shadow = QImage();
    update();
    colorChanged(m_color);
}

void ShadowEffect::resetColor()
{
    setColor(QColor());
}

QRectF ShadowEffect::boundingRectFor(const QRectF &rect) const
{
    qreal padding = m_blurRadius * 2;
    return rect.united(
        rect.translated(m_xOffset, m_yOffset)
        .adjusted(-padding, -padding, padding, padding)
        );
}

QImage ShadowEffect::generateShadow(const QPixmap &px) const
{
    if (px.isNull()) {
        return QImage();
    }

    QColor color = m_color.isValid() ? m_color : computeColorFromSource();

    QImage tmp(px.size(), QImage::Format_ARGB32_Premultiplied);
    tmp.fill(0);
    if (m_blurRadius > 0) {
        QPainter painter(&tmp);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.drawPixmap(m_xOffset, m_yOffset, px);
        painter.end();
        Plasma::PaintUtils::shadowBlur(tmp, m_blurRadius, color);
    } else {
        QPainter painter(&tmp);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.fillRect(m_xOffset, m_yOffset, px.width(), px.height(), color);
        painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        painter.drawPixmap(m_xOffset, m_yOffset, px);
    }
    return tmp;
}

void ShadowEffect::draw(QPainter *painter)
{
    if (m_blurRadius < 0 && qFuzzyIsNull(m_xOffset) && qFuzzyIsNull(m_yOffset)) {
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


/*****************************************************************************
 * UGLYNESS
 *
 * If the shadow color is not defined, we want to compute it from the color of
 * the item on which the effect is applied.
 *
 * To do so we need to reach the item from the effect.
 *
 * QGraphicsEffect has an internal, but public, "source()" method, which
 * returns a pointer to a QGraphicsEffectSource, which has a "graphicsItem()"
 * public method.
 *
 * QGraphicsEffectSource is not in any installed header, so we need to declare
 * it. Since "graphicsItem()" is not virtual we can escape with declaring a
 * fake QGraphicsEffectSource class, containing only the graphicsItem() method.
 *
 * The original class is defined in Qt4 in src/gui/effects/qgraphicseffect_p.h
 *****************************************************************************/
class QGraphicsEffectSource
{
public:
    const QGraphicsItem *graphicsItem() const;
};


QColor ShadowEffect::computeColorFromSource() const
{
    const QGraphicsItem *item = source()->graphicsItem();
    if (!item) {
        kWarning() << "No source item!";
        return Qt::black;
    }
    const QGraphicsObject *obj = item->toGraphicsObject();
    if (!obj) {
        kWarning() << "Source is not a QGraphicsObject";
        return Qt::black;
    }
    QVariant variant = obj->property("color");
    if (!variant.canConvert<QColor>()) {
        kWarning() << "Source has no 'color' property, or its 'color' property is not a QColor!";
        return Qt::black;
    }
    int value = variant.value<QColor>().value();
    return value >= 128 ? Qt::black : Qt::white;
}

#include <shadoweffect.moc>
