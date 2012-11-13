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
// Self
#include <abstractsource.h>

// Local

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
