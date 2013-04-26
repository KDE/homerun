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
#ifndef ACTION_H
#define ACTION_H

// Local

// Qt

// KDE
#include <KAction>

/**
 * Thin QML wrapper around KAction
 *
 * To be used with ActionManager
 * Be sure to define the action objectName(), otherwise ActionManager
 * integration will fail!
 */
class Action : public KAction
{
    Q_OBJECT
    /**
     * Define shortcut keys. Each element of the list is one shortcut.
     * Each element is a string using the same format as
     * QKeySequence::fromString()
     *
     * Examples:
     * - "Ctrl+PgUp"
     * - "Ctrl+PgDown"
     * - "Alt+S"
     * - "Ctrl+Alt+Left"
     */
    Q_PROPERTY(QStringList keys READ keys WRITE setKeys NOTIFY keysChanged)
public:
    explicit Action(QObject *parent = 0);
    ~Action();

    QStringList keys() const;
    void setKeys(const QStringList &keys);

Q_SIGNALS:
    void keysChanged();

private Q_SLOTS:
    void finishInit();

private:
    QStringList m_keys;
};

#endif /* ACTION_H */
