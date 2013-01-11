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
#ifndef ABSTRACTSOURCE_H
#define ABSTRACTSOURCE_H

// Local
#include <homerun_export.h>

// Qt
#include <QObject>
#include <QVariant>

// KDE
#include <KSharedConfig>

class QAbstractItemModel;

namespace Homerun {

class SourceRegistry;
class SourceRegistryPrivate;

class SourceConfigurationWidget;

class AbstractSourcePrivate;

/**
 * Base class to implement Homerun sources
 *
 * The job of a source is to allow Homerun to instantiate models.
 * The models are what show the items in Homerun tabs.
 *
 * You don't need to inherit from this class if you want to write a simple
 * source, look at the SimpleSource class instead.
 *
 * A source can be configurable, to make it configurable, reimplement
 * isConfigurable() to return true, and reimplement createConfigurationWidget()
 * to return a SourceConfigurationWidget initialized from the source
 * configuration.
 *
 * @see @ref homerunmodel
 */
class HOMERUN_EXPORT AbstractSource : public QObject
{
    Q_OBJECT
public:
    AbstractSource(QObject *parent, const QVariantList &args = QVariantList());
    ~AbstractSource();

    /**
     * The main method. You must reimplement this method to create a model from
     * a configuration group.
     *
     * @param group The configuration group to read from
     * @return a Qt model, exposing Homerun roles and properties
     */
    virtual QAbstractItemModel *createModelFromConfigGroup(const KConfigGroup &group) = 0;

    /**
     * If you want your source to be usable from other sources using the
     * openSourceRequested() signal, you must reimplement this method.
     * It returns a null pointer by default.
     *
     * Note: it would have made more sense for args to be a QVariantHash as the
     * order of the arguments does not matter. It is a QVariantMap because of
     * a C++ -> QML -> C++ problem: When a QVariantHash is exposed to QML it is
     * turned into a JavaScript Object, but when this Object is passed back to
     * C++, it is turned into a QVariantMap, not a QVariantHash.
     *
     * @param args A map of named arguments
     * @return a Qt model, exposing Homerun roles and properties
     */
    virtual QAbstractItemModel *createModelFromArguments(const QVariantMap &args);

    /**
     * Indicates whether this source is configurable.
     *
     * When configuring Homerun tabs, Homerun shows a configure button for
     * configurable sources. When the configure button is clicked, Homerun calls
     * createConfigurationWidget() and shows a dialog with the returned widget.
     *
     * Default implementation returns false.
     *
     * @return Whether this source is configurable or not
     *
     */
    virtual bool isConfigurable() const;

    /**
     * Returns a widget to configure the source. See SourceConfigurationWidget
     * for more details.
     *
     * isConfigurable() must have been reimplemented to return true for this
     * method to be called.
     *
     * @param group The configuration group for this source
     * @return A SourceConfigurationWidget ready to be shown by Homerun
     */
    virtual SourceConfigurationWidget *createConfigurationWidget(const KConfigGroup &group);

    /**
     * Returns a KSharedConfig for Homerun configuration. You most likely do not
     * need this, configuration should usually be done through
     * SourceConfigurationWidget.
     *
     * @return A KSharedConfig for Homerun configuration
     */
    KSharedConfig::Ptr config() const;

private:
    AbstractSourcePrivate * const d;

    /**
     * @internal
     */
    void setConfig(const KSharedConfig::Ptr&);

    friend class SourceRegistry;
    friend class SourceRegistryPrivate;
};

/**
 * If your source just creates an argument-less model, you can use this simpler
 * template class.
 */
template<class T>
class HOMERUN_EXPORT SimpleSource : public AbstractSource
{
public:
    SimpleSource(QObject *parent, const QVariantList &args = QVariantList())
    : AbstractSource(parent, args)
    {}

    QAbstractItemModel *createModelFromConfigGroup(const KConfigGroup &/*group*/)
    {
        return new T;
    }
};

} // namespace Homerun

/**
 * libName: the library name without the "homerun_source_" prefix
 * className: the class name of the source
 */
#define HOMERUN_EXPORT_SOURCE(libName, className) \
    K_PLUGIN_FACTORY(factory, registerPlugin<className>();) \
    K_EXPORT_PLUGIN(factory("homerun_source_" #libName))

#endif /* ABSTRACTSOURCE_H */
