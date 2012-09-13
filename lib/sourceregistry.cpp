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
#include <KPluginFactory>
#include <KPluginLoader>
#include <KServiceTypeTrader>

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

struct PluginInfo
{
    QStringList sources;
    KService::Ptr service;

    PluginInfo(KService::Ptr ptr)
    : service(ptr)
    {
        QVariant value = ptr->property("X-KDE-Homerun-Sources", QVariant::StringList);
        sources = value.toStringList();
    }
};

//- SourceRegistryPrivate -------------------------------------
struct SourceRegistryPrivate
{
    QHash<QString, QAbstractItemModel*> m_favoriteModels;
    QHash<QString, AbstractSource *> m_sources;
    KSharedConfig::Ptr m_config;

    QList<PluginInfo> m_pluginInfoList;

    void listSourcePlugins()
    {
        KService::List offers = KServiceTypeTrader::self()->query("Homerun/Source");
        Q_FOREACH(KService::Ptr ptr, offers) {
            m_pluginInfoList << PluginInfo(ptr);
        }
    }

    void loadPluginForSource(const QString &name)
    {
        // Look for a plugin providing a source named 'name'
        auto it = m_pluginInfoList.begin(), end = m_pluginInfoList.end();
        for (; it != end; ++it) {
            if (it->sources.contains(name)) {
                break;
            }
        }
        if (it == end) {
            return;
        }
        QStringList sourceNames = it->sources;
        KService::Ptr ptr = it->service;
        m_pluginInfoList.erase(it);

        // Create the plugin factory
        KPluginLoader loader(*ptr);
        KPluginFactory *factory = loader.factory();
        if (!factory) {
            kWarning() << "Failed to load plugin (desktop file: " << ptr->entryPath() << ", source:" << name << ")";
            kWarning() << loader.errorString();
            return;
        }

        // Create and register all sources provided by the plugin
        Q_FOREACH(const QString &sourceName, sourceNames) {
            AbstractSource *source = factory->create<AbstractSource>(sourceName);
            if (!source) {
                kWarning() << "Failed to create source from plugin (desktop file: " << ptr->entryPath() << ", source:" << name << ")";
                continue;
            }

            m_sources.insert(sourceName, source);
        }
    }
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

    Q_FOREACH(AbstractSource *source, d->m_sources) {
        source->init(this);
    }

    d->listSourcePlugins();
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
        d->loadPluginForSource(sourceId.name());
        source = d->m_sources.value(sourceId.name());
        if (!source) {
            kWarning() << "No source named" << sourceId.name();
            return 0;
        }
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
