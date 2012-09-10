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
#include <favoriteappsmodel.h>
#include <groupedservicemodel.h>
#include <placesmodel.h>
#include <powermodel.h>
#include <runnermodel.h>
#include <servicemodel.h>
#include <sessionmodel.h>

// KDE
#include <KDebug>

// Qt

//- AbstractSource --------------------------------------------
AbstractSource::AbstractSource(SourceRegistry *registry)
: QObject(registry)
, m_registry(registry)
{}

SourceRegistry *AbstractSource::registry() const
{
    return m_registry;
}

//- SimpleSource ----------------------------------------------
template<class T>
class SimpleSource : public AbstractSource
{
public:
    SimpleSource(SourceRegistry *registry)
    : AbstractSource(registry)
    {}

    QAbstractItemModel *modelForSource(const QString &args)
    {
        T* model = new T(registry());

        if (!args.isEmpty()) {
            if (model->metaObject()->indexOfProperty("arguments") >= 0) {
                model->setProperty("arguments", args);
            } else {
                kWarning() << "Trying to set arguments on a model which does not support arguments";
            }
        }

        return model;
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

    QAbstractItemModel *modelForSource(const QString &/* args */)
    {
        return m_model;
    }

private:
    QAbstractItemModel *m_model;
};

//- PlacesSource ----------------------------------------------
class PlacesSource : public AbstractSource
{
public:
    PlacesSource(SourceRegistry *registry)
    : AbstractSource(registry)
    {}

    QAbstractItemModel *modelForSource(const QString &args)
    {
        PlacesModel *model = new PlacesModel(registry());
        model->setRootModel(registry()->favoriteModel("place"));
        model->setArguments(args);
        return model;
    }
};

//- SourceRegistry --------------------------------------------
SourceRegistry::SourceRegistry(QObject *parent)
: QObject(parent)
{
    m_favoriteModels.insert("app", new FavoriteAppsModel(this));
    m_favoriteModels.insert("place", new FavoritePlacesModel(this));

    m_sources.insert("ServiceModel", new SimpleSource<ServiceModel>(this));
    m_sources.insert("GroupedServiceModel", new SimpleSource<GroupedServiceModel>(this));
    m_sources.insert("PlacesModel", new PlacesSource(this));
    m_sources.insert("FavoriteAppsModel", new SingletonSource(m_favoriteModels.value("app"), this));
    m_sources.insert("PowerModel", new SimpleSource<PowerModel>(this));
    m_sources.insert("SessionModel", new SimpleSource<SessionModel>(this));
    m_sources.insert("RunnerModel", new SimpleSource<RunnerModel>(this));
}

SourceRegistry::~SourceRegistry()
{
}

QObject *SourceRegistry::createModelForSource(const QString &sourceString)
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

    AbstractSource *source = m_sources.value(name);
    if (!source) {
        kWarning() << "No source named" << name;
        return 0;
    }
    model = source->modelForSource(args);
    Q_ASSERT(model);
    model->setObjectName(name);

    return model;
}

QVariantMap SourceRegistry::favoriteModels() const
{
    QVariantMap map;
    auto it = m_favoriteModels.constBegin(), end = m_favoriteModels.constEnd();
    for (; it != end; ++it) {
        map.insert(it.key(), QVariant::fromValue<QObject *>(it.value()));
    }
    return map;
}

QAbstractItemModel *SourceRegistry::favoriteModel(const QString &name) const
{
    return m_favoriteModels.value(name);
}

#include <sourceregistry.moc>
