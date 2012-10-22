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
#include <KPluginInfo>
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
            return sourceInfo->name;
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
            KPluginInfo pluginInfo(ptr);
            if (pluginInfo.pluginName().isEmpty()) {
                kWarning() << "Missing X-KDE-PluginInfo-Name key in" << ptr->entryPath();
                continue;
            }
            SourceInfo *sourceInfo = new SourceInfo;
            sourceInfo->service = ptr;
            sourceInfo->name = pluginInfo.pluginName();
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

    void registerSource(const QString &name, AbstractSource *source, const QString &visibleName, const QString &comment)
    {
        SourceInfo *info = new SourceInfo;
        info->name = name;
        info->visibleName = visibleName;
        info->source = source;
        info->comment = comment;
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

    AbstractSource *sourceBySourceString(const QString &sourceString)
    {
        bool ok;
        SourceId sourceId = SourceId::fromString(sourceString, &ok);
        if (!ok) {
            kWarning() << "Invalid sourceString" << sourceString;
            return 0;
        }
        return sourceByName(sourceId.name());
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

    d->registerSource("InstalledApps", new InstalledAppsSource(this),
        i18n("Installed Applications"),
        i18n("Browse installed applications by categories")
    );
    d->registerSource("GroupedInstalledApps", new GroupedInstalledAppsSource(this),
        i18n("All Installed Applications"),
        i18n("List all installed applications in one flat, grouped list")
    );
    d->registerSource("Dir", new DirSource(this),
        i18n("Folder"),
        i18n("List the content of a folder, let you browse into it")
    );
    d->registerSource("FavoritePlaces", new SingletonSource(d->m_favoriteModels.value("place"), this),
        i18n("Favorite Places"),
        i18n("Browse the content of your favorite places")
    );
    d->registerSource("FavoriteApps", new SingletonSource(d->m_favoriteModels.value("app"), this),
        i18n("Favorite Applications"),
        i18n("List your favorite applications")
    );
    d->registerSource("Power", new SimpleSource<PowerModel>(this),
        i18n("Power Management"),
        i18n("Provide buttons to suspend, hibernate, reboot or halt your computer")
    );
    d->registerSource("Session", new SimpleSource<SessionModel>(this),
        i18n("Session"),
        i18n("Provide buttons to lock the screen, log out, or switch to another user")
    );
    d->registerSource("Runner", new RunnerSource(this),
        i18n("KRunner"),
        i18n("Perform searchs using a selection of runners")
    );

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
    SourceInfo *info = d->m_sourceInfoByName.value(sourceId.name());
    if (!info) {
        kWarning() << "No source for" << sourceString;
        return QString();
    }
    return info->visibleName;
}

bool SourceRegistry::isSourceConfigurable(const QString &sourceString) const
{
    AbstractSource *source = d->sourceBySourceString(sourceString);
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
