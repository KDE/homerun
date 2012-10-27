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
#ifndef SOURCEREGISTRY_H
#define SOURCEREGISTRY_H

// Local
#include <homerun_export.h>

#include <abstractsourceregistry.h>

// Qt

// KDE

class QAbstractItemModel;

namespace Homerun {

class AbstractSource;
class SourceRegistryPrivate;

/**
 * The source registry. This class is responsible for loading source plugins
 * and instantiating source models.
 */
class HOMERUN_EXPORT SourceRegistry : public QObject, public AbstractSourceRegistry
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap favoriteModels READ favoriteModels CONSTANT)
    Q_PROPERTY(QString configFileName READ configFileName WRITE setConfigFileName NOTIFY configFileNameChanged)
public:
    explicit SourceRegistry(QObject *parent = 0);
    ~SourceRegistry();

    Q_INVOKABLE QObject *createModelForSource(const QString &sourceString, QObject *parent); // reimp
    KSharedConfig::Ptr config() const; //reimp

    QVariantMap favoriteModels() const;

    QAbstractItemModel *favoriteModel(const QString &name) const;

    QString configFileName() const;
    void setConfigFileName(const QString &name);

    //// Config API
    Q_INVOKABLE QObject *availableSourcesModel() const;

    Q_INVOKABLE QString visibleNameForSource(const QString &sourceString) const;

    Q_INVOKABLE bool isSourceConfigurable(const QString &sourceString) const;

    Q_INVOKABLE QObject *createConfigurationDialog(const QString &sourceString);

Q_SIGNALS:
    void configFileNameChanged(const QString &);

private:
    SourceRegistryPrivate * const d;
};

} // namespace Homerun

#endif /* SOURCEREGISTRY_H */
