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
// Self
#include <sourceconfigurationdialog.h>

// Local
#include <abstractsource.h>
#include <sourceconfigurationwidget.h>

// KDE

// Qt

namespace Homerun
{

SourceConfigurationDialog::SourceConfigurationDialog(AbstractSource *source, const KConfigGroup &group, QWidget *parent)
: KDialog(parent)
, m_sourceWidget(source->createConfigurationWidget(group))
{
    setMainWidget(m_sourceWidget);
}

SourceConfigurationDialog::~SourceConfigurationDialog()
{
}

void SourceConfigurationDialog::save()
{
    m_sourceWidget->save();
    m_sourceWidget->configGroup().sync();
}

} // namespace

#include <sourceconfigurationdialog.moc>
