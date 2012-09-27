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
#include <groupedinstalledappsmodel.h>
#include <libhomerun_config.h>
#include <placesmodel.h>
#include <powermodel.h>
#include <runnermodel.h>
#include <installedappsmodel.h>
#include <sessionmodel.h>
#include <sourceid.h>
#include <sourceconfigurationdialog.h>

// KDE
#include <KConfigGroup>
#include <KDebug>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KServiceTypeTrader>

// Qt
#include <QApplication>

namespace Homerun {

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

//- SourceInfo ------------------------------------------------
struct SourceInfo
{
    QString name;
    AbstractSource *source;
    KService::Ptr service;

    SourceInfo()
    : source(0)
    {}
};

//- AvailableSourcesModel -------------------------------------
class AvailableSourcesModel : public QAbstractListModel
{
public:
    enum {
        SourceIdRole = Qt::UserRole + 1
    };

    AvailableSourcesModel(const QList<SourceInfo *> &sources, QObject *parent)
    : QAbstractListModel(parent)
    , m_sourceInfos(sources)
    {
        QHash<int, QByteArray> roles;
        roles.insert(Qt::DisplayRole, "display");
        roles.insert(SourceIdRole, "sourceId");
        setRoleNames(roles);
    }

    int rowCount(const QModelIndex &parent) const
    {
        return parent.isValid() ? 0 : m_sourceInfos.size();
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        int row = index.row();
        if (row < 0 || row >= m_sourceInfos.size()) {
            return QVariant();
        }
        SourceInfo *sourceInfo = m_sourceInfos.at(row);
        switch (role) {
        case Qt::DisplayRole:
            return sourceInfo->name;
        case SourceIdRole:
            return sourceInfo->name;
        default:
            break;
        }
        return QVariant();
    }

private:
    const QList<SourceInfo *> &m_sourceInfos;
};

//- SourceRegistryPrivate -------------------------------------
struct SourceRegistryPrivate
{
    SourceRegistry *q;
    QHash<QString, QAbstractItemModel*> m_favoriteModels;

    QList<SourceInfo *> m_sourceInfos;
    QHash<QString, SourceInfo *> m_sourceInfoByName;

    AvailableSourcesModel *m_availableSourcesModel;
    KSharedConfig::Ptr m_config;

    void listSourcePlugins()
    {
        KService::List offers = KServiceTypeTrader::self()->query(
            "Homerun/Source",
            QString("[X-KDE-Homerun-APIVersion] == %1").arg(HOMERUN_API_VERSION)
            );
        Q_FOREACH(KService::Ptr ptr, offers) {
            QVariant value = ptr->property("X-KDE-PluginInfo-Name", QVariant::String);
            QString name = value.toString();
            if (name.isEmpty()) {
                kWarning() << "Missing X-KDE-PluginInfo-Name key in" << ptr->entryPath();
                continue;
            }
            SourceInfo *sourceInfo = new SourceInfo;
            sourceInfo->service = ptr;
            sourceInfo->name = name;
            registerSourceInfo(sourceInfo);
        }
    }

    void loadPluginForSourceInfo(SourceInfo *sourceInfo)
    {
        Q_ASSERT(sourceInfo->service);
        // Create the plugin factory
        KPluginLoader loader(*sourceInfo->service);
        KPluginFactory *factory = loader.factory();
        if (!factory) {
            kWarning() << "Failed to load plugin (desktop file: " << sourceInfo->service->entryPath() << ", source:" << sourceInfo->name << ")";
            kWarning() << loader.errorString();
            return;
        }

        // Create and register the source
        AbstractSource *source = factory->create<AbstractSource>();
        if (!source) {
            kWarning() << "Failed to create source from plugin (desktop file: " << sourceInfo->service->entryPath() << ", source:" << sourceInfo->name << ")";
            return;
        }
        sourceInfo->source = source;
        source->init(q);
    }

    void registerSource(const QString &name, AbstractSource *source)
    {
        SourceInfo *info = new SourceInfo;
        info->name = name;
        info->source = source;
        registerSourceInfo(info);
    }

    void registerSourceInfo(SourceInfo *info)
    {
        m_sourceInfos << info;
        m_sourceInfoByName.insert(info->name, info);
    }

    AbstractSource *sourceByName(const QString &name)
    {
        SourceInfo *sourceInfo = m_sourceInfoByName.value(name);
        if (!sourceInfo) {
            kWarning() << "No source named" << name;
            return 0;
        }
        if (sourceInfo->source) {
            return sourceInfo->source;
        }
        loadPluginForSourceInfo(sourceInfo);
        if (!sourceInfo->source) {
            kWarning() << "Failed to load source for" << name;
            return 0;
        }
        return sourceInfo->source;
    }
};

//- SourceRegistry --------------------------------------------
SourceRegistry::SourceRegistry(QObject *parent)
: QObject(parent)
, d(new SourceRegistryPrivate)
{
    d->q = this;
    d->m_availableSourcesModel = new AvailableSourcesModel(d->m_sourceInfos, this);

    d->m_favoriteModels.insert("app", new FavoriteAppsModel(this));
    d->m_favoriteModels.insert("place", new FavoritePlacesModel(this));

    d->registerSource("InstalledApps", new InstalledAppsSource(this));
    d->registerSource("GroupedInstalledApps", new GroupedInstalledAppsSource(this));
    d->registerSource("Dir", new DirSource(this));
    d->registerSource("FavoritePlaces", new SingletonSource(d->m_favoriteModels.value("place"), this));
    d->registerSource("FavoriteApps", new SingletonSource(d->m_favoriteModels.value("app"), this));
    d->registerSource("Power", new SimpleSource<PowerModel>(this));
    d->registerSource("Session", new SimpleSource<SessionModel>(this));
    d->registerSource("Runner", new RunnerSource(this));

    Q_FOREACH(SourceInfo *sourceInfo, d->m_sourceInfos) {
        sourceInfo->source->init(this);
    }

    d->listSourcePlugins();
}

SourceRegistry::~SourceRegistry()
{
    qDeleteAll(d->m_sourceInfos);
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

    AbstractSource *source = d->sourceByName(sourceId.name());
    if (!source) {
        return 0;
    }

    QAbstractItemModel *model = source->createModel(sourceId.arguments());
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

QString SourceRegistry::configFileName() const
{
    return d->m_config ? d->m_config->name() : QString();
}

void SourceRegistry::setConfigFileName(const QString &name)
{
    if (d->m_config && d->m_config->name() == name) {
        return;
    }
    d->m_config = KSharedConfig::openConfig(name);
    configFileNameChanged(name);
}

QAbstractItemModel *SourceRegistry::availableSourcesModel() const
{
    return d->m_availableSourcesModel;
}

QString SourceRegistry::visibleNameForSource(const QString &sourceString) const
{
    bool ok;
    SourceId sourceId = SourceId::fromString(sourceString, &ok);
    if (!ok) {
        kWarning() << "Invalid sourceString" << sourceString;
        return QString();
    }
    // FIXME: Get info from plugin desktop file
    return sourceId.name();
}

bool SourceRegistry::isSourceConfigurable(const QString &sourceString) const
{
    bool ok;
    SourceId sourceId = SourceId::fromString(sourceString, &ok);
    if (!ok) {
        kWarning() << "Invalid sourceString" << sourceString;
        return false;
    }
    AbstractSource *source = d->sourceByName(sourceId.name());
    if (!source) {
        kWarning() << "No source for" << sourceString;
        return false;
    }
    return source->isConfigurable();
}

QObject *SourceRegistry::createConfigurationDialog(const QString &sourceString)
{
    bool ok;
    SourceId sourceId = SourceId::fromString(sourceString, &ok);
    if (!ok) {
        kWarning() << "Invalid sourceString" << sourceString;
        return 0;
    }
    AbstractSource *source = d->sourceByName(sourceId.name());
    if (!source) {
        kWarning() << "No source for" << sourceString;
        return 0;
    }
    return new SourceConfigurationDialog(source, sourceId, QApplication::activeWindow());
}

} // namespace Homerun

#include <sourceregistry.moc>
