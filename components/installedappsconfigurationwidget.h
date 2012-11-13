/*
Copyright 2012 Aurélien Gâteau <agateau@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef INSTALLEDAPPSCONFIGURATIONWIDGET_H
#define INSTALLEDAPPSCONFIGURATIONWIDGET_H

// Local
#include <sourceconfigurationwidget.h>

// Qt

// KDE
#include <KServiceGroup>

class QStandardItem;
class QStandardItemModel;

class Ui_InstalledAppsConfigurationWidget;

namespace Homerun
{

/**
 * Configuration widget for the InstalledApps source
 */
class InstalledAppsConfigurationWidget : public SourceConfigurationWidget
{
    Q_OBJECT
public:
    explicit InstalledAppsConfigurationWidget(const KConfigGroup &group);
    ~InstalledAppsConfigurationWidget();

    void save(); // reimp

private:
    Ui_InstalledAppsConfigurationWidget *m_ui;
    QStandardItemModel *m_model;

    /**
     * Fill m_model, returns the item which corresponds to defaultEntryPath
     */
    QStandardItem *fillModel(const QString &defaultEntryPath);
};

} // namespace Homerun

#endif /* INSTALLEDAPPSCONFIGURATIONWIDGET_H */
