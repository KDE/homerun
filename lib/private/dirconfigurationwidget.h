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
#ifndef DIRCONFIGURATIONWIDGET_H
#define DIRCONFIGURATIONWIDGET_H

// Local
#include <sourceconfigurationwidget.h>
#include <sourceid.h>

// Qt

// KDE

class KUrlRequester;

/**
 * Configuration widget for the Dir source
 */
class DirConfigurationWidget : public Homerun::SourceConfigurationWidget
{
    Q_OBJECT
public:
    DirConfigurationWidget(const Homerun::SourceArguments &args);
    Homerun::SourceArguments arguments() const;

private:
    KUrlRequester *m_requester;
};

#endif /* DIRCONFIGURATIONWIDGET_H */
