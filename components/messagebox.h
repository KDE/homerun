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
#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

// Local

// Qt
#include <QObject>

// KDE
#include <KMessageBox>

/**
 * Wrapper around KMessageBox
 */
class MessageBox : public QObject
{
    Q_OBJECT
    Q_ENUMS(ButtonCode)
public:
    explicit MessageBox(QObject *parent = 0);

    enum ButtonCode {
        Ok = KMessageBox::Ok,
        Cancel = KMessageBox::Cancel,
        Yes = KMessageBox::Yes,
        No = KMessageBox::No,
        Continue = KMessageBox::Continue,
    };

    /**
     * Return value is a MessageBox::ButtonCode
     */
    Q_INVOKABLE int warningContinueCancel(
        const QString &text,
        const QString &caption = QString(),
        const QString &continueText = QString(),
        const QString &cancelText = QString()
    );
};

#endif /* MESSAGEBOX_H */
