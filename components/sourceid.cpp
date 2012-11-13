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
// Self
#include <sourceid.h>

// Local

// KDE
#include <KDebug>

// Qt
#include <QStringList>

static const char SOURCE_NAME_SEPARATOR = ':';

#define ARG_SEPARATOR ';'
#define ARG_SEPARATOR_STR ";"
static const char ARG_KEY_VALUE_SEPARATOR = '=';

//- SourceId ----------------------------------
QString SourceId::name() const
{
    return m_name;
}

void SourceId::setName(const QString &name)
{
    m_name = name;
}

SourceArguments &SourceId::arguments()
{
    return m_arguments;
}

const SourceArguments &SourceId::arguments() const
{
    return m_arguments;
}

QString SourceId::toString() const
{
    if (m_arguments.isEmpty()) {
        return m_name;
    }
    return m_name % SOURCE_NAME_SEPARATOR % m_arguments.toString();
}

SourceId SourceId::fromString(const QString &sourceString, bool *ok)
{
    *ok = true;
    SourceId sourceId;

    int idx = sourceString.indexOf(SOURCE_NAME_SEPARATOR);
    if (idx == -1) {
        sourceId.setName(sourceString);
        return sourceId;
    }

    sourceId.m_name = sourceString.left(idx);
    QString args = sourceString.mid(idx + 1);

    sourceId.m_arguments = SourceArguments::fromString(args, ok);
    if (!*ok) {
        return SourceId();
    }
    return sourceId;
}

bool SourceId::isValid() const
{
    return !m_name.isEmpty();
}

//- SourceArguments ---------------------------
static QString escapeValue(const QString &src)
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

SourceArguments SourceArguments::fromString(const QString &str, bool *ok)
{
    Q_ASSERT(ok);
    *ok = true;
    SourceArguments args;
    if (str.isEmpty()) {
        return args;
    }

    QStringList tokens = split(str);
    Q_FOREACH(const QString &token, tokens) {
        int idx = token.indexOf(ARG_KEY_VALUE_SEPARATOR);
        if (idx == -1) {
            kWarning() << "Invalid argument" << token << "in" << str;
            *ok = false;
            return SourceArguments();
        }
        QString key = token.left(idx);
        QString value = token.mid(idx + 1);
        args.insert(key, value);
    }
    return args;
}

SourceArguments &SourceArguments::add(const QString &key, const QString &value)
{
    insert(key, value);
    return *this;
}

QString SourceArguments::toString() const
{
    QStringList lst;
    auto it = constBegin(), end = constEnd();
    for (; it != end ; ++it) {
        lst << it.key() % ARG_KEY_VALUE_SEPARATOR % escapeValue(it.value());
    }
    return lst.join(ARG_SEPARATOR_STR);
}
