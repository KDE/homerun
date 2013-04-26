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
#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

// Local
#include <action.h>

// Qt
#include <QDeclarativeListProperty>

// KDE
#include <KActionCollection>
#include <KConfigGroup>

class QTimer;

/**
 * Handles a list of Action items, make it possible to configure their shortcuts.
 *
 * Quite similar to KActionCollection, but does not directly expose a
 * KActionCollection.
 *
 * Usage:
 * ActionManager {
 *     id: actionManager
 *     configFileName: "myapprc"
 *     Action {
 *         ...
 *     }
 *     Action {
 *         ...
 *     }
 *     ...
 * }
 *
 * To configure action shortcuts:
 *
 * actionManager.configure()
 */
class ActionManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QDeclarativeListProperty<Action> actions READ actions)
    Q_PROPERTY(QString configFileName READ configFileName WRITE setConfigFileName NOTIFY configFileNameChanged)
    Q_CLASSINFO("DefaultProperty", "actions")
public:
    explicit ActionManager(QObject *parent = 0);
    ~ActionManager();

    QDeclarativeListProperty<Action> actions();

    Q_INVOKABLE void configure();

    QString configFileName() const;
    void setConfigFileName(const QString &name);

Q_SIGNALS:
    void configFileNameChanged();

private Q_SLOTS:
    void readSettings();

private:
    QTimer *m_readSettingsTimer;
    KActionCollection *m_collection;
    QDeclarativeListProperty<Action> m_list;
    QString m_configFileName;
    KConfigGroup m_configGroup;

    void addAction(Action *action);
    static void append(QDeclarativeListProperty<Action> *property, Action *action);
};

#endif /* ACTIONMANAGER_H */
