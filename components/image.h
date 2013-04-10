/*
Copyright 2013 Aurélien Gâteau <agateau@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef IMAGE_H
#define IMAGE_H

// Local

// Qt
#include <QDeclarativeItem>

// KDE

/**
 * An item which shows images whose sources can be either the name of an icon
 * or a QIcon.
 */
class Image : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(QVariant source READ source WRITE setSource NOTIFY sourceChanged)
public:
    explicit Image(QDeclarativeItem *parent = 0);
    ~Image();

    QVariant source() const;
    void setSource(const QVariant &value);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

Q_SIGNALS:
    void sourceChanged();

protected:
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    QVariant m_source;
    QPixmap m_pixmap;

    void reload();
};

#endif /* IMAGE_H */
