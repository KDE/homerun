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
#ifndef SOURCEID_H
#define SOURCEID_H

// Local

// Qt
#include <QHash>
#include <QString>

// KDE

/**
 * The arguments of a source. Used within a SourceId instance.
 */
class SourceArguments : public QHash<QString, QString>
{
public:
    SourceArguments &add(const QString &key, const QString &value);

    QString toString() const;

    static SourceArguments fromString(const QString &, bool *ok);
};

/**
 * Represents a source and its optional arguments
 */
class SourceId
{
public:
    void setName(const QString &);
    QString name() const;
    SourceArguments &arguments();
    const SourceArguments &arguments() const;

    QString toString() const;

    static SourceId fromString(const QString &, bool *ok);

    bool isValid() const;

private:
    QString m_name;
    SourceArguments m_arguments;
};

#endif /* SOURCEID_H */
