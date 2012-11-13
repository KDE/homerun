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
#ifndef RUNNERCONFIGURATIONWIDGET_H
#define RUNNERCONFIGURATIONWIDGET_H

// Local
#include <sourceconfigurationwidget.h>
#include <ui_runnerconfigurationwidget.h>

// Qt

// KDE

namespace Homerun
{

/**
 * A configuration widget for the Runner source
 */
class RunnerConfigurationWidget : public SourceConfigurationWidget, private Ui_RunnerConfigurationWidget
{
    Q_OBJECT
public:
    explicit RunnerConfigurationWidget(const KConfigGroup &);

    void save(); // reimp
};

} // namespace

#endif /* RUNNERCONFIGURATIONWIDGET_H */
