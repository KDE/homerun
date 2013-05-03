/*
Copyright 2013 Aurélien Gâteau <agateau@kde.org>
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
#ifndef CHANGENOTIFIER_H
#define CHANGENOTIFIER_H

// Local

// Qt
#include <QObject>
#include <QStringList>

// KDE

class QTimer;

namespace Homerun {

/**
 * Monitor application changes from KSycoca
 */
class ChangeNotifier : public QObject
{
    Q_OBJECT
public:
    explicit ChangeNotifier(QObject *parent = 0);
    ~ChangeNotifier();

    bool eventFilter(QObject *watched, QEvent *event);

Q_SIGNALS:
    void changeDetected(bool needsReload);

private Q_SLOTS:
    void checkSycocaChanges(const QStringList &changes);
    void timeout();

private:
    QTimer *mTimer;
    QStringList mWatchedProps;
};

} // namespace Homerun

#endif /* CHANGENOTIFIER_H */
