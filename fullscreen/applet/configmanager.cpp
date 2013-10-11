/*
Copyright 2012 Aurélien Gâteau <agateau@kde.org>

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
#include <configmanager.h>

// Local
#include <configkeys.h>
#include <ui_generalconfigpage.h>

// KDE
#include <KConfigDialog>

// Qt

ConfigManager::ConfigManager(const KConfigGroup &group, KConfigDialog *dialog)
: QObject(dialog)
, m_group(group)
, m_generalUi(new Ui_GeneralConfigPage)
{
    // General page
    QWidget *generalPage = new QWidget;
    m_generalUi->setupUi(generalPage);

    m_generalUi->iconButton->setIcon(
        m_group.readEntry(CFG_LAUNCHER_ICON_KEY, CFG_LAUNCHER_ICON_DEFAULT)
        );

    connect(m_generalUi->iconButton, SIGNAL(iconChanged(QString)), dialog, SLOT(settingsModified()));
    dialog->addPage(generalPage, i18n("General"), "homerun");

    // Global connections
    connect(dialog, SIGNAL(applyClicked()), SLOT(writeConfig()));
    connect(dialog, SIGNAL(okClicked()), SLOT(writeConfig()));
}

ConfigManager::~ConfigManager()
{
    delete m_generalUi;
}

void ConfigManager::writeConfig()
{
    m_group.writeEntry(CFG_LAUNCHER_ICON_KEY, m_generalUi->iconButton->icon());
    configChanged();
}

#include <configmanager.moc>
