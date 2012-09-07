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
#include <groupedservicemodel.h>
#include <placesmodel.h>
#include <powermodel.h>
#include <runnermodel.h>
#include <servicemodel.h>
#include <sessionmodel.h>

// KDE
#include <KDebug>

// Qt

SourceRegistry::SourceRegistry(QObject *parent)
: QObject(parent)
{
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
    if (modelName == "ServiceModel") {
        model = new ServiceModel(this);
    } else if (modelName == "GroupedServiceModel") {
        model = new GroupedServiceModel(this);
    } else if (modelName == "PlacesModel") {
        PlacesModel* placesModel = new PlacesModel(this);
        placesModel->setRootModel(m_favoriteModels["place"].value<QObject*>());
        model = placesModel;
    } else if (modelName == "FavoriteAppsModel") {
        return m_favoriteModels["app"].value<QObject*>();
    } else if (modelName == "PowerModel") {
        model = new PowerModel(this);
    } else if (modelName == "SessionModel") {
        model = new SessionModel(this);
    } else if (modelName == "RunnerModel") {
        model = new RunnerModel(this);
    } else {
        kWarning() << "Unknown model type" << modelName;
        return 0;
    }
    Q_ASSERT(model);
    model->setObjectName(source);

    if (!modelArgs.isEmpty()) {
        if (model->metaObject()->indexOfProperty("arguments") >= 0) {
            model->setProperty("arguments", modelArgs);
        } else {
            kWarning() << "Trying to set arguments on model " << modelName << ", which does not support arguments";
        }
    }

    return model;
}

QVariantMap SourceRegistry::favoriteModels() const
{
    return m_favoriteModels;
}

void SourceRegistry::setFavoriteModels(const QVariantMap &models)
{
    m_favoriteModels = models;
    favoriteModelsChanged();
}

#include <sourceregistry.moc>
