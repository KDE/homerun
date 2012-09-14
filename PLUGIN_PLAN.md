# Architecture overview

## SourceRegistry

Holds all sources. One instance of it is created in main.qml.

Responsabilities:

- holds the favorite models
- instantiate source models given their arguments
- instantiate a SourceConfigurator to configure a model source arguments

Methods:

    QObject *createModelForSource(const QString &sourceString, QObject *parent);

Properties:

    QVariantMap favoriteModels

## AbstractSource

Methods:

    QAbstractItemModel *createModel(const SourceArguments::Hash &)
    SourceConfigurator *createConfigurator() const

A source can be either static (bundled in homeruncomponents.so) or dynamic:
implemented in a plugin.

## Source plugin

A source plugin is a standard KDE plugin which implements the KPluginFactory
class.

If a plugin wants to provide multiple sources, it must implement
KPluginFactory::create() and return different sources depending on the value of
the first element of the "args" parameter, which will be the source name.

Each source plugin should have its own .desktop file installed as:

    $KDEDIR/share/kde4/services/homerun-source-$name.desktop

Custom keys for the .desktop file:

- int X-KDE-Homerun-APIVersion
- QStringList X-KDE-Homerun-Sources

## SourceConfigurator

Created by an AbstractSource to let the user configure the source: provides a
UI to adjust the source arguments.

Must be able to:

- create a set of widgets
- initialize widgets from source arguments
- create source arguments from widgets

API:
    void setupUi(QWidget *)
    void setSourceArguments(const SourceArguments &)
    SourceArguments sourceArguments() const

# TODO

## Modify SourceRegistry to support plugins

## Implement the SourceConfigurator class
