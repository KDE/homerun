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
#include <clocks.h>

// Homerun
#include <abstractsource.h>

// KDE
#include <KDebug>
#include <KGlobal>
#include <KLocale>
#include <KPluginFactory>
#include <KSystemTimeZones>
#include <KTimeZone>

// Qt
#include <QPainter>
#include <QTimer>

typedef Homerun::SimpleSource<ClocksModel> ClocksSource;
HOMERUN_EXPORT_SOURCE(clocks, ClocksSource)

class ClockItem : public QStandardItem
{
public:
    ClockItem(const KTimeZone &zone)
    : m_timeZone(zone)
    {
        setText(m_timeZone.name());
        update();
    }

    void update()
    {
        QDateTime utc = QDateTime::currentDateTimeUtc();
        QDateTime local = m_timeZone.toZoneTime(utc);
        QString timeString = KGlobal::locale()->formatTime(local.time());

        QPixmap pix(128, 128);
        pix.fill(Qt::transparent);
        QPainter painter(&pix);

        painter.setRenderHint(QPainter::Antialiasing);
        painter.translate(.5, .5);
        painter.setBrush(Qt::lightGray);
        painter.setPen(Qt::black);
        painter.drawRoundedRect(pix.rect().adjusted(0, 0, -1, -1), 12, 12);

        QFont font;
        font.setPointSize(24);
        painter.setFont(font);
        painter.translate(-.5, -.5);
        painter.drawText(pix.rect(), Qt::AlignCenter, timeString);
        painter.end();

        setIcon(pix);
    }

private:
    KTimeZone m_timeZone;
};

ClocksModel::ClocksModel()
{
    load();
    scheduleUpdate();
}

void ClocksModel::load()
{
    clear();
    KTimeZone local = KSystemTimeZones::local();
    KTimeZone oman  = KSystemTimeZones::zone("Asia/Muscat");
    QList<KTimeZone> zones = QList<KTimeZone>() << local << oman;

    Q_FOREACH(const KTimeZone &zone, zones) {
        appendRow(new ClockItem(zone));
    }
}

void ClocksModel::scheduleUpdate()
{
    // Start timer when second == 0
    QTime now = QTime::currentTime();
    int delay = (60 - now.second()) * 1000;
    QTimer::singleShot(delay, this, SLOT(update()));
}

void ClocksModel::update()
{
    for (int idx = 0; idx < rowCount(); ++idx) {
        QStandardItem *itm = item(idx);
        static_cast<ClockItem *>(itm)->update();
    }
    scheduleUpdate();
}

bool ClocksModel::trigger(int /*row*/)
{
    return false;
}

QString ClocksModel::name() const
{
    return i18n("Clocks");
}

int ClocksModel::count() const
{
    return rowCount(QModelIndex());
}

#include <clocks.moc>
