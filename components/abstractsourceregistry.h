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
#ifndef ABSTRACTSOURCEREGISTRY_H
#define ABSTRACTSOURCEREGISTRY_H

// Local

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
class AbstractSourceRegistry : public QObject
{
    Q_OBJECT
public:
    AbstractSourceRegistry(QObject *parent = 0);
    ~AbstractSourceRegistry();

    virtual QObject *createModelFromConfigGroup(const QString &sourceId, const KConfigGroup &configGroup, QObject *parent);
};

} // namespace Homerun

#endif /* ABSTRACTSOURCEREGISTRY_H */
