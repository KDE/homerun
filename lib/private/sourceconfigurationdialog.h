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
#ifndef SOURCECONFIGURATIONDIALOG_H
#define SOURCECONFIGURATIONDIALOG_H

// Local

// Qt

// KDE
#include <KDialog>

class KConfigGroup;

namespace Homerun
{

class AbstractSource;
class SourceConfigurationWidget;

/**
 * A dialog to configure a source
 */
class SourceConfigurationDialog : public KDialog
{
    Q_OBJECT
public:
    explicit SourceConfigurationDialog(AbstractSource *source, const KConfigGroup &group, QWidget *parent);
    ~SourceConfigurationDialog();

    Q_INVOKABLE void save();

private:
    SourceConfigurationWidget *m_sourceWidget;
};

} // namespace

#endif /* SOURCECONFIGURATIONDIALOG_H */
