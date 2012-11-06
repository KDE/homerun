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
#include <abstractsource.h>

// Local
#include <sourceregistry.h>

// KDE

// Qt

namespace Homerun {

struct AbstractSourcePrivate
{
    KSharedConfig::Ptr m_config;
};

AbstractSource::AbstractSource(QObject *parent, const QVariantList &/*args*/)
: QObject(parent)
, d(new AbstractSourcePrivate)
{
}

AbstractSource::~AbstractSource()
{
    delete d;
}

QAbstractItemModel *AbstractSource::createModelFromArguments(const QVariantMap &)
{
    return 0;
}

bool AbstractSource::isConfigurable() const
{
    return false;
}

SourceConfigurationWidget *AbstractSource::createConfigurationWidget(const KConfigGroup &)
{
    return 0;
}

KSharedConfig::Ptr AbstractSource::config() const
{
    return d->m_config;
}

void AbstractSource::setConfig(const KSharedConfig::Ptr& config)
{
    d->m_config = config;
}

} // namespace Homerun

#include <abstractsource.moc>
