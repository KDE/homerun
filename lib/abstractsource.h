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
#ifndef ABSTRACTSOURCE_H
#define ABSTRACTSOURCE_H

// Local
#include <homerun_export.h>

// Qt
#include <QObject>
#include <QVariant>

// KDE

class QAbstractItemModel;

class SourceArguments;
class SourceRegistry;

class AbstractSourcePrivate;

/**
 * Base class to implement homerun sources
 */
class HOMERUN_EXPORT AbstractSource : public QObject
{
    Q_OBJECT
public:
    AbstractSource(QObject *parent, const QVariantList &args = QVariantList());
    ~AbstractSource();

    virtual void init(SourceRegistry *registry);

    SourceRegistry *registry() const;

    virtual QAbstractItemModel *createModel(const SourceArguments &args) = 0;

private:
    AbstractSourcePrivate * const d;
};

/**
 * If your source just creates an argument-less model, you can use this simpler
 * template class.
 */
template<class T>
class HOMERUN_EXPORT SimpleSource : public AbstractSource
{
public:
    SimpleSource(QObject *parent, const QVariantList &args = QVariantList())
    : AbstractSource(parent, args)
    {}

    QAbstractItemModel *createModel(const SourceArguments &/*args*/)
    {
        return new T;
    }
};


#endif /* ABSTRACTSOURCE_H */
