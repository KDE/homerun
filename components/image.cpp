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
// Self
#include <image.h>

// Local

// KDE
#include <KDebug>
#include <KIcon>

// Qt
#include <QPainter>
#include <QStyleOptionGraphicsItem>

Image::Image(QDeclarativeItem *parent)
: QDeclarativeItem(parent)
{
    setFlag(QGraphicsItem::ItemHasNoContents, false);
}

Image::~Image()
{
}

QVariant Image::source() const
{
    return m_source;
}

void Image::setSource(const QVariant &source)
{
    if (source == m_source) {
        return;
    }
    m_source = source;
    reload();
    sourceChanged();
}

void Image::reload()
{
    const int extent = qMin(width(), height());
    if (m_source.canConvert<QString>()) {
        KIcon icon(m_source.toString());
        m_pixmap = icon.pixmap(extent);
    } else if (m_source.canConvert<QIcon>()) {
        QIcon icon = m_source.value<QIcon>();
        m_pixmap = icon.pixmap(extent);
    } else {
        if (m_source.isValid()) {
            kWarning() << "Don't know how to handle source" << m_source;
        }
        m_pixmap = QPixmap();
    }
    // Always update, even if we do not have valid content. If we had content
    // before, we need to erase it.
    update();
}

void Image::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (newGeometry.size() != oldGeometry.size()) {
        reload();
    }
}

void Image::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    if (m_pixmap.isNull()) {
        return;
    }
    const QRect &rect = option->rect;
    painter->drawPixmap(
        rect.x() + (rect.width() - m_pixmap.width())/ 2,
        rect.y() + (rect.height() - m_pixmap.height()) / 2,
        m_pixmap);
}


#include <image.moc>
