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
#include <sourceregistry.h>

// Local
#include <abstractsource.h>
#include <customtypes.h>
#include <libhomerun_config.h>
#include <sourceconfigurationdialog.h>

#include <sources/dir/dirmodel.h>
#include <sources/favorites/favoriteappsmodel.h>
#include <sources/favorites/favoriteplacesmodel.h>
#include <sources/installedapps/installedappsmodel.h>
#include <sources/installedapps/groupedinstalledappsmodel.h>
#include <sources/installedapps/filterableinstalledappsmodel.h>
#include <sources/recentapps/recentappsmodel.h>
#include <sources/power/powermodel.h>
#include <sources/power/combinedpowersessionmodel.h>
#include <sources/runners/singlerunnermodel.h>
#include <sources/runners/runnermodel.h>
#include <sources/session/openedsessionsmodel.h>
#include <sources/session/sessionmodel.h>

// KDE
#include <KConfigGroup>
#include <KDebug>
#include <KPluginFactory>
#include <KPluginInfo>
#include <KPluginLoader>
#include <KServiceTypeTrader>
#include <Plasma/PluginLoader>

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

    QAbstractItemModel *createModelFromConfigGroup(const KConfigGroup &/* group */)
    {
        return m_model;
    }

private:
    QAbstractItemModel *m_model;
};

//- SourceInfo ------------------------------------------------
struct SourceInfo
{
    QString id;
    QString visibleName;
    QString comment;
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
        SourceIdRole = Qt::UserRole + 1,
        CommentRole
    };

    AvailableSourcesModel(const QList<SourceInfo *> &sources, QObject *parent)
    : QAbstractListModel(parent)
    , m_sourceInfos(sources)
    {
        QHash<int, QByteArray> roles;
        roles.insert(Qt::DisplayRole, "display");
        roles.insert(SourceIdRole, "sourceId");
        roles.insert(CommentRole, "comment");
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
            return sourceInfo->visibleName;
        case SourceIdRole:
            return sourceInfo->id;
        case CommentRole:
            return sourceInfo->comment;
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
    QHash<QString, SourceInfo *> m_sourceInfoById;

    AvailableSourcesModel *m_availableSourcesModel;
    KSharedConfig::Ptr m_config;

    void listSourcePlugins()
    {
        KService::List offers = KServiceTypeTrader::self()->query(
            "Homerun/Source",
            QString("[X-KDE-Homerun-APIVersion] == %1").arg(HOMERUN_API_VERSION)
            );

        Q_FOREACH(KService::Ptr ptr, offers) {
            KPluginInfo pluginInfo(ptr);
            if (pluginInfo.pluginName().isEmpty()) {
                kWarning() << "Missing X-KDE-PluginInfo-Name key in" << ptr->entryPath();
                continue;
            }
            SourceInfo *sourceInfo = new SourceInfo;
            sourceInfo->service = ptr;
            sourceInfo->id = pluginInfo.pluginName();
            sourceInfo->visibleName = pluginInfo.name();
            sourceInfo->comment = pluginInfo.comment();
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
            kWarning() << "Failed to load plugin (desktop file: " << sourceInfo->service->entryPath() << ", source:" << sourceInfo->id << ")";
            kWarning() << loader.errorString();
            return;
        }

        // Create the source
        AbstractSource *source = factory->create<AbstractSource>();
        if (!source) {
            kWarning() << "Failed to create source from plugin (desktop file: " << sourceInfo->service->entryPath() << ", source:" << sourceInfo->id << ")";
            return;
        }
        source->setConfig(m_config);
        sourceInfo->source = source;
    }

    void registerSource(const QString &id, AbstractSource *source, const QString &visibleName, const QString &comment)
    {
        SourceInfo *info = new SourceInfo;
        info->id = id;
        info->visibleName = visibleName;
        info->source = source;
        info->comment = comment;
        registerSourceInfo(info);
    }

    void registerSourceInfo(SourceInfo *info)
    {
        m_sourceInfos << info;
        m_sourceInfoById.insert(info->id, info);
    }

    AbstractSource *sourceById(const QString &id)
    {
        SourceInfo *sourceInfo = m_sourceInfoById.value(id);
        if (!sourceInfo) {
            kWarning() << "No source named" << id;
            return 0;
        }
        if (sourceInfo->source) {
            return sourceInfo->source;
        }
        loadPluginForSourceInfo(sourceInfo);
        if (!sourceInfo->source) {
            kWarning() << "Failed to load source for" << id;
            return 0;
        }
        return sourceInfo->source;
    }

    void registerSingleRunnerSources()
    {
        KPluginInfo::List list = Plasma::PluginLoader::pluginLoader()->listRunnerInfo();

        // FIXME: SC 4.13 replaced Nepomuk with Baloo for desktop search. Modifications
        // to this logic skip registering the "nepomuksearch" runner when "baloosearch"
        // is found and alias the latter to the former in the source registry to keep
        // existing config files working. This can be dropped again once we depend on a
        // SC version guaranteed to have Baloo around.
        bool gotBaloo = false;

        Q_FOREACH(const KPluginInfo &info, list) {
            if (info.pluginName() == "baloosearch") {
                gotBaloo = true;
            }
        }

        Q_FOREACH(const KPluginInfo &info, list) {
            if (!info.property("X-Plasma-AdvertiseSingleRunnerQueryMode").toBool()
                || (gotBaloo && info.pluginName() == "nepomuksearch")) {
                continue;
            }
            QString runnerId = info.pluginName();
            SingleRunnerSource *source = new SingleRunnerSource(runnerId, q);
            QString name = i18n("Runner: %1", info.name());
            registerSource("SingleRunner/" + runnerId, source, name, info.comment());

            if (runnerId == "baloosearch") {
                registerSource("SingleRunner/nepomuksearch", source, name, info.comment());
            }
        }
    }
};

//- SourceRegistry --------------------------------------------
SourceRegistry::SourceRegistry(QObject *parent)
: AbstractSourceRegistry(parent)
, d(new SourceRegistryPrivate)
{
    d->q = this;
    d->m_availableSourcesModel = new AvailableSourcesModel(d->m_sourceInfos, this);

    d->m_favoriteModels.insert("app", new FavoriteAppsModel(this));
    d->m_favoriteModels.insert("place", new FavoritePlacesModel(this));

    d->registerSource("InstalledApps", new InstalledAppsSource(this),
        i18n("Installed Applications"),
        i18n("Browse installed applications by categories")
    );
    d->registerSource("GroupedInstalledApps", new GroupedInstalledAppsSource(this),
        i18n("All Installed Applications"),
        i18n("List all installed applications in a flat list, grouped by categories")
    );
    d->registerSource("FilterableInstalledApps", new FilterableInstalledAppsSource(this),
        i18n("All Installed Applications With Filters"),
        i18n("List all installed applications and filter via the sidebar")
    );
    d->registerSource("RecentApps", new RecentAppsSource(this),
        i18n("Recent Applications"),
        i18n("List the most recently launched applications")
    );
    d->registerSource("Dir", new DirSource(this),
        i18n("Folder"),
        i18n("List the content of a folder and let you browse into sub-folders")
    );
    d->registerSource("FavoritePlaces", new SingletonSource(d->m_favoriteModels.value("place"), this),
        i18n("Favorite Places"),
        i18n("Browse the content of your favorite places")
    );
    d->registerSource("FavoriteApps", new SingletonSource(d->m_favoriteModels.value("app"), this),
        i18n("Favorite Applications"),
        i18n("List applications marked as favorite")
    );
    d->registerSource("Power", new SimpleSource<PowerModel>(this),
        i18n("Power Management"),
        i18n("Provide buttons to suspend, hibernate, reboot or halt your computer")
    );
    d->registerSource("Session", new SimpleSource<SessionModel>(this),
        i18n("Session"),
        i18n("Provide buttons to lock the screen, log out, or switch to another user")
    );
    d->registerSource("OpenedSessions", new SimpleSource<OpenedSessionsModel>(this),
        i18n("Opened Sessions"),
        i18n("Provide buttons to switch to opened sessions")
    );
    d->registerSource("CombinedPowerSession", new CombinedPowerSessionSource(this),
        i18n("Power / Session"),
        i18n("Provide buttons to manage your workspace session and the power state of your computer")
    );
    d->registerSource("Runner", new RunnerSource(this),
        i18n("KRunner"),
        i18n("Perform searchs using a selection of runners")
    );

    d->registerSingleRunnerSources();

    d->listSourcePlugins();
}

SourceRegistry::~SourceRegistry()
{
    qDeleteAll(d->m_sourceInfos);
    delete d;
}

QObject *SourceRegistry::createModelFromArguments(const QString &sourceId, const QVariantMap &sourceArguments, QObject *parent)
{
    // Get source
    AbstractSource *source = d->sourceById(sourceId);
    if (!source) {
        kWarning() << "Invalid sourceId in group (sourceId=" << sourceId << ")";
        return 0;
    }

    // Create model
    QAbstractItemModel *model = source->createModelFromArguments(sourceArguments);
    if (!model) {
        kWarning() << "Failed to create model";
        return 0;
    }
    model->setObjectName(sourceId);

    // If the model already has a parent, then don't change it.
    // This is used by singleton sources to keep their model alive.
    if (!model->parent()) {
        model->setParent(parent);
    }

    return model;
}

QObject *SourceRegistry::createModelFromConfigGroup(const QString &sourceId, const KConfigGroup &group, QObject *parent)
{
    // Get source
    AbstractSource *source = d->sourceById(sourceId);
    if (!source) {
        kWarning() << "Invalid sourceId in group (sourceId=" << sourceId << ")";
        return 0;
    }

    // Create model
    QAbstractItemModel *model = source->createModelFromConfigGroup(group);
    if (!model) {
        kWarning() << "Failed to create model from group" << group.name();
        return 0;
    }
    model->setObjectName(sourceId);

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
    Q_FOREACH(SourceInfo *sourceInfo, d->m_sourceInfos) {
        AbstractSource *source = sourceInfo->source;
        if (source) {
            source->setConfig(d->m_config);
        }
    }
    configFileNameChanged(name);
}

QObject *SourceRegistry::availableSourcesModel() const
{
    return d->m_availableSourcesModel;
}

QString SourceRegistry::visibleNameForSource(const QString &sourceId) const
{
    SourceInfo *info = d->m_sourceInfoById.value(sourceId);
    if (!info) {
        kWarning() << "No source for" << sourceId;
        return QString();
    }
    return info->visibleName;
}

bool SourceRegistry::isSourceConfigurable(const QString &sourceId) const
{
    AbstractSource *source = d->sourceById(sourceId);
    if (!source) {
        kWarning() << "No source for" << sourceId;
        return false;
    }
    return source->isConfigurable();
}

QObject *SourceRegistry::createConfigurationDialog(const QString &sourceId, const QVariant &configGroupVariant) const
{
    AbstractSource *source = d->sourceById(sourceId);
    if (!source) {
        kWarning() << "No source for" << sourceId;
        return 0;
    }

    KConfigGroup *sourceGroup = configGroupVariant.value<KConfigGroup*>();
    Q_ASSERT(sourceGroup);
    Q_ASSERT(sourceGroup->isValid());

    return new SourceConfigurationDialog(source, *sourceGroup, QApplication::activeWindow());
}

} // namespace Homerun

#include <sourceregistry.moc>
