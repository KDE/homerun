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
#ifndef SOURCEPLUGINLOADER_H
#define SOURCEPLUGINLOADER_H

// Local

// Qt
#include <QObject>
#include <QVariant>

// KDE

class QAbstractItemModel;

/**
 *
 */
class SourcePluginLoader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap favoriteModels READ favoriteModels WRITE setFavoriteModels NOTIFY favoriteModelsChanged)
public:
    explicit SourcePluginLoader(QObject *parent = 0);
    ~SourcePluginLoader();

    Q_INVOKABLE QObject *createModelForSource(const QString &source);

    QVariantMap favoriteModels() const;
    void setFavoriteModels(const QVariantMap &models);

Q_SIGNALS:
    void favoriteModelsChanged();

private:
    QVariantMap m_favoriteModels;
};

#endif /* SOURCEPLUGINLOADER_H */
