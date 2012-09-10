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

//- AbstractSourcePlugin --------------------------------------------
AbstractSourcePlugin::AbstractSourcePlugin(SourceRegistry *registry)
: QObject(registry)
, m_registry(registry)
{}

SourceRegistry *AbstractSourcePlugin::registry() const
{
    return m_registry;
}

//- SimpleSourcePlugin ----------------------------------------------
template<class T>
class SimpleSourcePlugin : public AbstractSourcePlugin
{
public:
    SimpleSourcePlugin(SourceRegistry *registry)
    : AbstractSourcePlugin(registry)
    {}

    QAbstractItemModel *modelForSource(const QString &name, const QString &args)
    {
        T* model = new T(registry());

        if (!args.isEmpty()) {
            if (model->metaObject()->indexOfProperty("arguments") >= 0) {
                model->setProperty("arguments", args);
            } else {
                kWarning() << "Trying to set arguments on model " << name << ", which does not support arguments";
            }
        }

        return model;
    }
};

//- SingletonSourcePlugin -------------------------------------------
class SingletonSourcePlugin : public AbstractSourcePlugin
{
public:
    SingletonSourcePlugin(QAbstractItemModel *model, SourceRegistry *registry)
    : AbstractSourcePlugin(registry)
    , m_model(model)
    {}

    QAbstractItemModel *modelForSource(const QString &/*name*/, const QString &/* args */)
    {
        return m_model;
    }

private:
    QAbstractItemModel *m_model;
};

//- PlacesSourcePlugin ----------------------------------------------
class PlacesSourcePlugin : public AbstractSourcePlugin
{
public:
    PlacesSourcePlugin(SourceRegistry *registry)
    : AbstractSourcePlugin(registry)
    {}

    QAbstractItemModel *modelForSource(const QString &/*name*/, const QString &args)
    {
        PlacesModel *model = new PlacesModel(registry());
        model->setRootModel(registry()->favoriteModel("place"));
        model->setArguments(args);
        return model;
    }
};

//- SourceRegistry --------------------------------------------------
SourceRegistry::SourceRegistry(QObject *parent)
: QObject(parent)
{
    m_favoriteModels.insert("app", new FavoriteAppsModel(this));
    m_favoriteModels.insert("place", new FavoritePlacesModel(this));

    m_pluginForSource.insert("ServiceModel", new SimpleSourcePlugin<ServiceModel>(this));
    m_pluginForSource.insert("GroupedServiceModel", new SimpleSourcePlugin<GroupedServiceModel>(this));
    m_pluginForSource.insert("PlacesModel", new PlacesSourcePlugin(this));
    m_pluginForSource.insert("FavoriteAppsModel", new SingletonSourcePlugin(m_favoriteModels.value("app"), this));
    m_pluginForSource.insert("PowerModel", new SimpleSourcePlugin<PowerModel>(this));
    m_pluginForSource.insert("SessionModel", new SimpleSourcePlugin<SessionModel>(this));
    m_pluginForSource.insert("RunnerModel", new SimpleSourcePlugin<RunnerModel>(this));
}

SourceRegistry::~SourceRegistry()
{
}

QObject *SourceRegistry::createModelForSource(const QString &source)
{
    QString modelName;
    QString modelArgs;
    int idx = source.indexOf(':');
    if (idx > 0) {
        modelName = source.left(idx);
        modelArgs = source.mid(idx + 1);
    } else {
        modelName = source;
    }

    QAbstractItemModel *model = 0;

    AbstractSourcePlugin *plugin = m_pluginForSource.value(modelName);
    if (!plugin) {
        kWarning() << "No plugin provides a source named" << modelName;
        return 0;
    }
    model = plugin->modelForSource(modelName, modelArgs);
    Q_ASSERT(model);
    model->setObjectName(source);

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
