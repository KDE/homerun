/*
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
#include <globalsettings.h>

// Local

// KDE
#include <KConfigGroup>
#include <KSharedConfig>

GlobalSettings::GlobalSettings(QObject *parent) : QObject(parent)
{
}

GlobalSettings::~GlobalSettings()
{
}

QString GlobalSettings::configFileName() const
{
    return m_configFileName;
}

void GlobalSettings::setConfigFileName(const QString &name)
{
    if (m_configFileName == name) {
        return;
    }

    m_configFileName = name;
    m_configGroup = KSharedConfig::openConfig(m_configFileName)->group("Global");
    emit configFileNameChanged();
    emit showActionListOverlayChanged();
    emit showDesktopToolBoxChanged();
}

bool GlobalSettings::showActionListOverlay() const
{
    if (!m_configGroup.isValid()) {
        return false;
    }
    return m_configGroup.readEntry<bool>("ShowActionListOverlay", false);
}

void GlobalSettings::setShowActionListOverlay(bool show)
{
    if (show == showActionListOverlay()) {
        return;
    }

    if (m_configGroup.isValid()) {
        m_configGroup.writeEntry("ShowActionListOverlay", show);
        m_configGroup.sync();
        emit showActionListOverlayChanged();
    }
}

bool GlobalSettings::showDesktopToolBox() const
{
    if (!m_configGroup.isValid()) {
        return false;
    }
    return m_configGroup.readEntry<bool>("ShowDesktopToolBox", false);
}

void GlobalSettings::setShowDesktopToolBox(bool show)
{
    if (show == showDesktopToolBox()) {
        return;
    }

    if (m_configGroup.isValid()) {
        m_configGroup.writeEntry("ShowDesktopToolBox", show);
        m_configGroup.sync();
        emit showDesktopToolBoxChanged();
    }
}

#include <globalsettings.moc>
