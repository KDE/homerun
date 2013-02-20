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
#include <sessionswatcher.h>

// Local

// KDE

// Qt
#include <QTimer>

/*
 * Polling sucks, but I don't know a better way given the wild jungle of
 * display managers available. Let's poll only every 30 seconds to avoid
 * draining batteries.
 * Eventually support for being notified about session changes should be
 * implemented at least for consolekit and systemd-logind
 */
static const int POLL_INTERVAL = 30 * 1000;

SessionsWatcher::SessionsWatcher(QObject *parent)
: QObject(parent)
{
    QTimer *timer = new QTimer(this);
    timer->setInterval(POLL_INTERVAL);
    connect(timer, SIGNAL(timeout()), SLOT(checkSessions()));
    timer->start();

    QMetaObject::invokeMethod(this, "checkSessions", Qt::QueuedConnection);
}

// SessEnt has no operator==() :/
inline bool sameSession(const SessEnt &s1, const SessEnt &s2)
{
#define CHECK_FIELD(f) if (s1.f != s2.f) { return false; }
    CHECK_FIELD(display)
    CHECK_FIELD(from)
    CHECK_FIELD(user)
    CHECK_FIELD(session)
    CHECK_FIELD(vt)
    CHECK_FIELD(self)
    CHECK_FIELD(tty)
#undef CHECK_FIELD
    return true;
}

void SessionsWatcher::checkSessions()
{
    SessList newSessions;
    m_displayManager.localSessions(newSessions);
    if (m_sessions.count() != newSessions.count()) {
        m_sessions = newSessions;
        sessionsChanged();
        return;
    }
    auto oldIt = m_sessions.constBegin(), oldEnd = m_sessions.constEnd();
    auto newIt = newSessions.constBegin();
    for (; oldIt != oldEnd; ++oldIt, ++newIt) {
        if (!sameSession(*oldIt, *newIt)) {
            m_sessions = newSessions;
            sessionsChanged();
            return;
        }
    }
}

SessList SessionsWatcher::sessions() const
{
    return m_sessions;
}



#include <sessionswatcher.moc>
