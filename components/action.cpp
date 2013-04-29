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
#include <QWidget>

Action::Action(QObject *parent)
: KAction(parent)
{
    connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), SLOT(finishInit()));
    finishInit();
}

void Action::finishInit()
{
    QWidget *widget = QApplication::activeWindow();
    if (!widget) {
        return;
    }
    widget->addAction(this);
    disconnect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), this, SLOT(finishInit()));
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

#include <action.moc>
