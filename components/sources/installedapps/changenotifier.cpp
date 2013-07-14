/*
Copyright 2013 Aurélien Gâteau <agateau@kde.org>
Copyright 2013 Eike Hein <hein@kde.org>

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
#include <changenotifier.h>

// Local

// KDE
#include <KSycoca>
#include <KDebug>

// Qt
#include <QApplication>
#include <QDynamicPropertyChangeEvent>
#include <QTimer>

namespace Homerun {

ChangeNotifier::ChangeNotifier(QObject *parent)
: QObject(parent)
, mTimer(new QTimer(this))
{
    // Use a timer to aggregate multiple KSycoca changes
    mTimer->setInterval(100);
    mTimer->setSingleShot(true);
    connect(mTimer, SIGNAL(timeout()), SLOT(timeout()));

    connect(KSycoca::self(), SIGNAL(databaseChanged(QStringList)), SLOT(checkSycocaChanges(QStringList)));

    mWatchedProps << "appletContainmentId" << "appletContainmentMutable"
        << "desktopContainmentId" << "desktopContainmentMutable";
    qApp->installEventFilter(this);
}

ChangeNotifier::~ChangeNotifier()
{
}

bool ChangeNotifier::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::DynamicPropertyChange
        && mWatchedProps.contains(static_cast<QDynamicPropertyChangeEvent *>(event)->propertyName())) {
        emit changeDetected(false);
    }

    return QObject::eventFilter(watched, event);
}

void ChangeNotifier::checkSycocaChanges(const QStringList &changes)
{
    if (changes.contains("services") || changes.contains("apps") || changes.contains("xdgdata-apps")) {
        mTimer->start();
    }
}

void ChangeNotifier::timeout()
{
    emit changeDetected(true);
}

} // namespace Homerun

#include <changenotifier.moc>
