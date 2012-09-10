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

// Qt
#include <QObject>
#include <QVariant>

// KDE

class QAbstractItemModel;

class SourceRegistry;

class AbstractSourcePlugin : public QObject
{
public:
    AbstractSourcePlugin(SourceRegistry *registry);

    SourceRegistry *registry() const;

    virtual QAbstractItemModel *modelForSource(const QString &name, const QString &args) = 0;

private:
    SourceRegistry *m_registry;
};

/**
 *
 */
class SourceRegistry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap favoriteModels READ favoriteModels CONSTANT)
public:
    explicit SourceRegistry(QObject *parent = 0);
    ~SourceRegistry();

    Q_INVOKABLE QObject *createModelForSource(const QString &source);

    QVariantMap favoriteModels() const;

    QAbstractItemModel *favoriteModel(const QString &name) const;

private:
    QHash<QString, QAbstractItemModel*> m_favoriteModels;

    QHash<QString, AbstractSourcePlugin *> m_pluginForSource;
};

#endif /* SOURCEREGISTRY_H */
