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
#include <sourceregistry.h>

// Local
#include <abstractsource.h>
#include <favoriteappsmodel.h>
#include <groupedservicemodel.h>
#include <placesmodel.h>
#include <powermodel.h>
#include <runnermodel.h>
#include <servicemodel.h>
#include <sessionmodel.h>
#include <sourceid.h>

// KDE
#include <KConfigGroup>
#include <KDebug>

// Qt

//- SingletonSource -------------------------------------------
class SingletonSource : public AbstractSource
{
public:
    SingletonSource(QAbstractItemModel *model, SourceRegistry *registry)
    : AbstractSource(registry)
    , m_model(model)
    {}

    QAbstractItemModel *createModel(const SourceArguments &/* args */)
    {
        return m_model;
    }

private:
    QAbstractItemModel *m_model;
};

//- SourceRegistryPrivate -------------------------------------
struct SourceRegistryPrivate
{
    QHash<QString, QAbstractItemModel*> m_favoriteModels;
    QHash<QString, AbstractSource *> m_sources;
    KSharedConfig::Ptr m_config;
};

//- SourceRegistry --------------------------------------------
SourceRegistry::SourceRegistry(QObject *parent)
: QObject(parent)
, d(new SourceRegistryPrivate)
{
    d->m_config = KSharedConfig::openConfig("homerunrc");
    d->m_favoriteModels.insert("app", new FavoriteAppsModel(this));
    d->m_favoriteModels.insert("place", new FavoritePlacesModel(this));

    d->m_sources.insert("Service", new ServiceSource(this));
    d->m_sources.insert("GroupedService", new GroupedServiceSource(this));
    d->m_sources.insert("Dir", new DirSource(this));
    d->m_sources.insert("FavoritePlaces", new SingletonSource(d->m_favoriteModels.value("place"), this));
    d->m_sources.insert("FavoriteApps", new SingletonSource(d->m_favoriteModels.value("app"), this));
    d->m_sources.insert("Power", new SimpleSource<PowerModel>(this));
    d->m_sources.insert("Session", new SimpleSource<SessionModel>(this));
    d->m_sources.insert("Runner", new RunnerSource(this));
}

SourceRegistry::~SourceRegistry()
{
    delete d;
}

QObject *SourceRegistry::createModelForSource(const QString &sourceString, QObject *parent)
{
    bool ok;
    SourceId sourceId = SourceId::fromString(sourceString, &ok);
    if (!ok) {
        kWarning() << "Invalid sourceString" << sourceString;
        return 0;
    }

    QAbstractItemModel *model = 0;

    AbstractSource *source = d->m_sources.value(sourceId.name());
    if (!source) {
        kWarning() << "No source named" << sourceId.name();
        return 0;
    }
    model = source->createModel(sourceId.arguments());
    if (!model) {
        kWarning() << "Failed to create model from" << sourceString;
        return 0;
    }
    model->setObjectName(sourceString);

    // If the model already has a parent, then don't change it.
    // This is used by singleton sources to keep their model alive.
    if (!model->parent()) {
        model->setParent(parent);
    }

    return model;
}

QVariantMap SourceRegistry::favoriteModels() const
{
    QVariantMap map;
    auto it = d->m_favoriteModels.constBegin(), end = d->m_favoriteModels.constEnd();
    for (; it != end; ++it) {
        map.insert(it.key(), QVariant::fromValue<QObject *>(it.value()));
    }
    return map;
}

QAbstractItemModel *SourceRegistry::favoriteModel(const QString &name) const
{
    return d->m_favoriteModels.value(name);
}

KSharedConfig::Ptr SourceRegistry::config() const
{
    return d->m_config;
}

#include <sourceregistry.moc>
