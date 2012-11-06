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
#ifndef ABSTRACTSOURCEREGISTRY_H
#define ABSTRACTSOURCEREGISTRY_H

// Local
#include <homerun_export.h>

// Qt
#include <QObject>

// KDE

class KConfigGroup;

namespace Homerun
{

/**
 * Defines the API of the SourceRegistry. Make it possible to mock the
 * SourceRegistry in unit-tests.
 *
 * Methods of this class cannot be pure virtual because AbstractSourceRegistry
 * needs to be declared as a QML component to be usable in TabModel.
 */
class HOMERUN_EXPORT AbstractSourceRegistry : public QObject
{
    Q_OBJECT
public:
    AbstractSourceRegistry(QObject *parent = 0);
    ~AbstractSourceRegistry();

    virtual QObject *createModelFromConfigGroup(const QString &sourceId, const KConfigGroup &configGroup, QObject *parent);
};

} // namespace Homerun

#endif /* ABSTRACTSOURCEREGISTRY_H */
