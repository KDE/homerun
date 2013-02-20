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
#include <openedsessionsmodel.h>

// Local
#include <sessionswatcher.h>

// KDE
#include <KLocale>

// Qt
#include <QTimer>

namespace Homerun
{

class SwitchSessionItem : public StandardItem
{
public:
    SwitchSessionItem(KDisplayManager *manager, const SessEnt &session)
    : m_displayManager(manager)
    , m_vt(session.vt)
    {
        QString user, location;
        KDisplayManager::sess2Str2(session, user, location);
        setText(user + QString(" (Ctrl+Alt+F%1)").arg(session.vt));
        if (session.user.isEmpty() && session.session.isEmpty()) {
            setIconName("preferences-system-login");
        } else {
            setIconName("user-identity");
        }
    }

    bool trigger(const QString &/*actionId*/, const QVariant &/*actionArgument*/) override
    {
        m_displayManager->lockSwitchVT(m_vt);
        return true;
    }

private:
    KDisplayManager *m_displayManager;
    int m_vt;
};

OpenedSessionsModel::OpenedSessionsModel(QObject *parent)
: StandardItemModel(parent)
, m_watcher(new SessionsWatcher(this))
{
    setName(i18n("Opened Sessions"));
    connect(m_watcher, SIGNAL(sessionsChanged()), SLOT(refresh()));
}

OpenedSessionsModel::~OpenedSessionsModel()
{
}

void OpenedSessionsModel::refresh()
{
    clear();

    SessList sessions = m_watcher->sessions();

    Q_FOREACH(const SessEnt &session, sessions) {
        if (!session.vt || session.self) {
            continue;
        }

        appendRow(new SwitchSessionItem(&m_displayManager, session));
    }
}

} // namespace

#include <openedsessionsmodel.moc>
