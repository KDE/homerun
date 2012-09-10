# Refactor sources to use JSON to define arguments

# Design a SourceConfigurator interface

Must be able to:

- create a set of widgets
- initialize widgets from source arguments
- create source arguments from widgets

    class SourceConfigurator : public QObject
        SourceConfig(QObject *)
        void setupUi(QWidget *)
        void setSourceArguments(const SourceArguments::Hash &)
        SourceArguments::Hash sourceArguments() const

# Design a AbstractSource interface

Must be able to:

- instantiate source models given their arguments
- instantiate a SourceConfigurator to configure a model source arguments

    class AbstractSource : public QObject
        QAbstractItemModel *modelForSource(const SourceArguments::Hash &)
        SourceConfigurator *createConfigurator() const

A source can be either static (bundled in homeruncomponents.so) or dynamic:
implemented in a plugin.

# Source plugins

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

Open questions:

- How to handle "global" config? (for example to read package management settings)
  It should probably be exposed from SourceRegistry

- Should we support singleton sources?

- How to expose favorite models?

- How to register a favorite model?

- Do we want multiple sources per plugin?
  (if not, then maybe SourcePlugin can be renamed to Source?)

# Port an existing source to SourcePlugin

# Create a SourceRegistry class, which can be instantiated from QML

Must be able to:

- instantiate a SourcePlugin given its name
- list available plugins

# Use SourceRegistry in TabContent.qml

# Port remaining existing sources to SourcePlugin

# Modify SourceRegistry to support plugins loaded from .so

# Real plugins

Either write a real plugin, implemented in its own .so or turn all|some existing sources into .so
