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
