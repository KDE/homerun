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
#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

// Qt
#include <QObject>

// KDE
#include <KConfigGroup>

class GlobalSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString configFileName READ configFileName WRITE setConfigFileName NOTIFY configFileNameChanged)
    Q_PROPERTY(bool showActionListOverlay READ showActionListOverlay WRITE setShowActionListOverlay NOTIFY showActionListOverlayChanged)
    Q_PROPERTY(bool showDesktopToolBox READ showDesktopToolBox WRITE setShowDesktopToolBox NOTIFY showDesktopToolBoxChanged)
public:
    explicit GlobalSettings(QObject *parent = 0);
    ~GlobalSettings();

    QString configFileName() const;
    void setConfigFileName(const QString &name);

    bool showActionListOverlay() const;
    void setShowActionListOverlay(bool show);

    bool showDesktopToolBox() const;
    void setShowDesktopToolBox(bool show);

Q_SIGNALS:
    void configFileNameChanged();
    void showActionListOverlayChanged();
    void showDesktopToolBoxChanged();

private:
    QString m_configFileName;
    KConfigGroup m_configGroup;
};

#endif /* GLOBALSETTINGS_H */
