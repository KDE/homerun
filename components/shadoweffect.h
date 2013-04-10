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
#ifndef SHADOWEFFECT_H
#define SHADOWEFFECT_H

// Qt
#include <QGraphicsEffect>

/**
 * An effect which draws a shadow behind an item, using
 * Plasma::PaintUtils::shadowBlur()
 *
 * If the color is not specified, it will attempt to compute the best
 * appropriate color, based on the item "color" property (if it has any).
 */
class ShadowEffect : public QGraphicsEffect
{
    Q_OBJECT
    Q_PROPERTY(qreal xOffset READ xOffset WRITE setXOffset NOTIFY xOffsetChanged)
    Q_PROPERTY(qreal yOffset READ yOffset WRITE setYOffset NOTIFY yOffsetChanged)
    Q_PROPERTY(qreal blurRadius READ blurRadius WRITE setBlurRadius NOTIFY blurRadiusChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
public:
    explicit ShadowEffect(QObject *parent = 0);
    ~ShadowEffect();

    QRectF boundingRectFor(const QRectF &rect) const; // reimp

    qreal xOffset() const;

    qreal yOffset() const;

    qreal blurRadius() const;
    QColor color() const;

    /**
     * Use this method if you set the color manually and now want ShadowEffect
     * to pick the color automatically, based on the color of the item on which
     * the effect is applied.
     */
    Q_INVOKABLE void resetColor();

public Q_SLOTS:
    void setXOffset(qreal dx);
    void setYOffset(qreal dy);
    void setBlurRadius(qreal blurRadius);
    void setColor(const QColor &color);

Q_SIGNALS:
    void xOffsetChanged(qreal);
    void yOffsetChanged(qreal);
    void blurRadiusChanged(qreal blurRadius);
    void colorChanged(const QColor &color);

protected:
    void draw(QPainter *painter); // reimp
    void sourceChanged(ChangeFlags flags); // reimp

private:
    qreal m_xOffset;
    qreal m_yOffset;
    qreal m_blurRadius;
    QColor m_color;

    QImage m_shadow;

    QImage generateShadow(const QPixmap &px) const;

    QColor computeColorFromSource() const;
};

#endif /* SHADOWEFFECT_H */
