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
#include <actionmanager.h>

// Local

// KDE
#include <KDebug>
#include <KSharedConfig>
#include <KShortcutsDialog>

// Qt
#include <QApplication>
#include <QTimer>

ActionManager::ActionManager(QObject *parent)
: QObject(parent)
, m_readSettingsTimer(new QTimer(this))
, m_collection(new KActionCollection(this))
, m_list(this, 0, append)
{
    m_readSettingsTimer->setSingleShot(true);
    m_readSettingsTimer->setInterval(0);
    connect(m_readSettingsTimer, SIGNAL(timeout()),
            SLOT(readSettings()));
}

ActionManager::~ActionManager()
{
}

QDeclarativeListProperty<Action> ActionManager::actions()
{
    return m_list;
}

void ActionManager::append(QDeclarativeListProperty<Action> *property, Action *action)
{
    static_cast<ActionManager *>(property->object)->addAction(action);
}

void ActionManager::addAction(Action *action)
{
    if (action->objectName().isEmpty()) {
        kWarning() << "Cannot add action" << action << "to collection: action.objectName has not been defined";
        return;
    }
    m_collection->addAction(action->objectName(), action);
    m_readSettingsTimer->start();
}

void ActionManager::configure()
{
    KShortcutsDialog::configure(m_collection,
                                KShortcutsEditor::LetterShortcutsAllowed,
                                QApplication::activeWindow(),
                                false /* bSaveSettings */);

    m_collection->writeSettings(&m_configGroup);
}

void ActionManager::readSettings()
{
    if (!m_configGroup.isValid()) {
        return;
    }
    m_collection->readSettings(&m_configGroup);
}

QString ActionManager::configFileName() const
{
    return m_configFileName;
}

void ActionManager::setConfigFileName(const QString &name)
{
    if (m_configFileName == name) {
        return;
    }
    m_configFileName = name;
    m_configGroup = KSharedConfig::openConfig(m_configFileName)->group("Shortcuts");
    m_readSettingsTimer->start();
    emit configFileNameChanged();
}

#include <actionmanager.moc>
