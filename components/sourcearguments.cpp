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
#include <sourcearguments.h>

// Local

// KDE
#include <KDebug>

// Qt

namespace SourceArguments
{

#define ARG_SEPARATOR ','
#define ARG_SEPARATOR_STR ","

QString escapeValue(const QString &src)
{
    QString dst = src;
    dst.replace('\\', "\\\\"); // must be done first
    dst.replace(ARG_SEPARATOR, "\\" ARG_SEPARATOR_STR);
    return dst;
}

static QStringList split(const QString &src)
{
    bool escaped = false;
    QStringList lst;
    QString token;
    auto it = src.constBegin(), end = src.constEnd();
    for (; it != end; ++it) {
        const QChar ch = *it;
        if (escaped) {
            escaped = false;
        } else if (ch == '\\') {
            escaped = true;
            continue;
        } else if (ch == ARG_SEPARATOR) {
            lst.append(token);
            token.clear();
            continue;
        }
        token.append(ch);
    }
    lst.append(token);
    return lst;
}

Hash parse(const QString &str)
{
    Hash args;
    if (str.isEmpty()) {
        return args;
    }

    QStringList tokens = split(str);
    Q_FOREACH(const QString &token, tokens) {
        int idx = token.indexOf('=');
        if (idx == -1) {
            kWarning() << "Ignoring invalid argument" << token << "in" << str;
            continue;
        }
        QString key = token.left(idx);
        QString value = token.mid(idx + 1);
        args.insert(key, value);
    }
    return args;
}

} // namespace
