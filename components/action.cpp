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
#include <action.h>

// Local

// KDE
#include <KDebug>

// Qt
#include <QApplication>
#include <QKeySequence>
#include <QList>
#include <QTimer>
#include <QWidget>

Action::Action(QObject *parent)
: KAction(parent)
{
    finishInit();
}

void Action::finishInit()
{
    QWidget *widget = QApplication::activeWindow();
    if (!widget) {
        QTimer::singleShot(100, this, SLOT(finishInit()));
        return;
    }
    widget->addAction(this);
}

Action::~Action()
{
}

QStringList Action::keys() const
{
    return m_keys;
}

void Action::setKeys(const QStringList &list)
{
    if (m_keys == list) {
        return;
    }
    QList<QKeySequence> sequenceList;
    Q_FOREACH(const QString &txt, list) {
        sequenceList.append(QKeySequence::fromString(txt));
    }
    setShortcuts(sequenceList);
    m_keys = list;
    keysChanged();
}

QString Action::name() const
{
    return m_name;
}

void Action::setName(const QString &value)
{
    if (m_name == value) {
        return;
    }
    m_name = value;
    nameChanged();
}

#include <action.moc>
