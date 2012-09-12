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

// KDE
#include <KConfigGroup>
#include <KDebug>

// Qt

//- SimpleSource ----------------------------------------------
template<class T>
class SimpleSource : public AbstractSource
{
public:
    SimpleSource(SourceRegistry *registry)
    : AbstractSource(registry)
    {}

    QAbstractItemModel *createModel(const QString &/*args*/)
    {
        return new T;
    }
};

//- SingletonSource -------------------------------------------
class SingletonSource : public AbstractSource
{
public:
    SingletonSource(QAbstractItemModel *model, SourceRegistry *registry)
    : AbstractSource(registry)
    , m_model(model)
    {}

    QAbstractItemModel *createModel(const QString &/* args */)
    {
        return m_model;
    }

private:
    QAbstractItemModel *m_model;
};

//- ServiceSource ---------------------------------------------
class ServiceSource : public AbstractSource
{
public:
    ServiceSource(SourceRegistry *registry)
    : AbstractSource(registry)
    {}

    QAbstractItemModel *createModel(const QString &args)
    {
        ServiceModel *model = new ServiceModel;

        KConfigGroup group(registry()->config(), "PackageManagement");
        model->setInstaller(group.readEntry("categoryInstaller"));

        model->setArguments(args);
        return model;
    }
};

//- GroupedServiceSource --------------------------------------
class GroupedServiceSource : public AbstractSource
{
public:
    GroupedServiceSource(SourceRegistry *registry)
    : AbstractSource(registry)
    {}

    QAbstractItemModel *createModel(const QString &/*args*/)
    {
        GroupedServiceModel *model = new GroupedServiceModel;

        KConfigGroup group(registry()->config(), "PackageManagement");
        model->setInstaller(group.readEntry("categoryInstaller"));

        return model;
    }
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
    d->m_sources.insert("Runner", new SimpleSource<RunnerModel>(this));
}

SourceRegistry::~SourceRegistry()
{
    delete d;
}

QObject *SourceRegistry::createModelForSource(const QString &sourceString, QObject *parent)
{
    QString name;
    QString args;
    int idx = sourceString.indexOf(':');
    if (idx > 0) {
        name = sourceString.left(idx);
        args = sourceString.mid(idx + 1);
    } else {
        name = sourceString;
    }

    QAbstractItemModel *model = 0;

    AbstractSource *source = d->m_sources.value(name);
    if (!source) {
        kWarning() << "No source named" << name;
        return 0;
    }
    model = source->createModel(args);
    Q_ASSERT(model);
    model->setObjectName(name);

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
