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
#ifndef SOURCEREGISTRY_H
#define SOURCEREGISTRY_H

// Local
#include <abstractsourceregistry.h>

// Qt

// KDE
#include <KSharedConfig>

class QAbstractItemModel;

namespace Homerun {

class AbstractSource;
class SourceRegistryPrivate;

/**
 * The source registry. This class is responsible for loading source plugins
 * and instantiating source models.
 */
class SourceRegistry : public AbstractSourceRegistry
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap favoriteModels READ favoriteModels CONSTANT)
    Q_PROPERTY(QString configFileName READ configFileName WRITE setConfigFileName NOTIFY configFileNameChanged)
public:
    explicit SourceRegistry(QObject *parent = 0);
    ~SourceRegistry();

    Q_INVOKABLE QObject *createModelFromArguments(const QString &sourceId, const QVariantMap &sourceArguments, QObject *parent);

    QObject *createModelFromConfigGroup(const QString &sourceId, const KConfigGroup &configGroup, QObject *parent); // reimp
    KSharedConfig::Ptr config() const;

    QVariantMap favoriteModels() const;

    QAbstractItemModel *favoriteModel(const QString &name) const;

    QString configFileName() const;
    void setConfigFileName(const QString &name);

    //// Config API
    Q_INVOKABLE QObject *availableSourcesModel() const;

    Q_INVOKABLE QString visibleNameForSource(const QString &sourceId) const;

    Q_INVOKABLE bool isSourceConfigurable(const QString &sourceId) const;

    Q_INVOKABLE QObject *createConfigurationDialog(const QString &sourceId, const QVariant &groupVariant) const;

Q_SIGNALS:
    void configFileNameChanged(const QString &);

private:
    SourceRegistryPrivate * const d;
};

} // namespace Homerun

#endif /* SOURCEREGISTRY_H */
