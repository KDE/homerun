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
#ifndef SOURCECONFIGURATIONWIDGET_H
#define SOURCECONFIGURATIONWIDGET_H

// Local
#include <homerun_export.h>
#include <sourceid.h>

// Qt
#include <QWidget>

// KDE

namespace Homerun
{

/**
 * Used by configurable sources to provide a configuration widget.
 */
class HOMERUN_EXPORT SourceConfigurationWidget : public QWidget
{
    Q_OBJECT
public:
    SourceConfigurationWidget(QWidget *parent = 0);
    virtual SourceArguments arguments() const = 0;
};

} // namespace

#endif /* SOURCECONFIGURATIONWIDGET_H */
