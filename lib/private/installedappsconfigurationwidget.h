/*
 * Copyright 2012 Aurélien Gâteau <agateau@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
