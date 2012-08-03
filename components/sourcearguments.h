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
#ifndef SOURCEARGUMENTS_H
#define SOURCEARGUMENTS_H

// Local

// Qt
#include <QHash>
#include <QString>
#include <QStringList>

// KDE

/**
 * Set of methods to serialize/deserialize arguments for Homerun sources
 *
 * Given the arguments:
 * - key1: value1
 * - key2: value2
 *
 * They get serialized into the string:
 * "key1=value1,key2=value2"
 *
 * ',' is used as a separator.
 * Keys may only contain alphanumeric characters.
 * A few characters are escaped in values:
 * - , => \'
 * - \ => \\
 */
namespace SourceArguments
{

/**
 * A dictionary of arguments
 */
typedef QHash<QString, QString> Hash;

/**
 * Escape a value so that it can be used in a string version of source arguments
 */
QString escapeValue(const QString &value);

/**
 * Parse a string version of source arguments into a hash
 */
Hash parse(const QString &str);

} // namespace

#endif /* SOURCEARGUMENTS_H */
