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
#ifndef SOURCECONFIGURATIONWIDGET_H
#define SOURCECONFIGURATIONWIDGET_H

// Local
#include <homerun_export.h>

// Qt
#include <QWidget>

// KDE

class KConfigGroup;

namespace Homerun
{

struct SourceConfigurationWidgetPrivate;

/**
 * Used by configurable sources to provide a configuration widget.
 */
class HOMERUN_EXPORT SourceConfigurationWidget : public QWidget
{
    Q_OBJECT
public:
    /**
     * Creates a configuration widget, initialized to show the configuration
     * from group
     *
     * @param group KConfigGroup to read configuration from
     * @param parent QWidget which will be the parent of this widget
     */
    SourceConfigurationWidget(const KConfigGroup &group, QWidget *parent = 0);
    ~SourceConfigurationWidget();

    /**
     * This method must save configuration changes to configGroup().
     * Homerun takes care of calling KConfig::sync() to ensure changes are
     * stored to disk.
     */
    virtual void save() = 0;

    /**
     * Returns the config group which was passed to the constructor.
     */
    KConfigGroup configGroup() const;

private:
    SourceConfigurationWidgetPrivate *const d;
};

} // namespace

#endif /* SOURCECONFIGURATIONWIDGET_H */
