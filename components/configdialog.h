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
#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

// Local

// Qt
#include <QObject>

// KDE

namespace Homerun
{
class SourceRegistry;
}

class TabModel;

/**
 * Shows a dialog to configure Homerun
 */
class ConfigDialog : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject *sourceRegistry READ sourceRegistry WRITE setSourceRegistry)
    Q_PROPERTY(QObject *tabModel READ tabModel WRITE setTabModel)
public:
    explicit ConfigDialog(QObject *parent = 0);
    ~ConfigDialog();

    QObject *sourceRegistry() const;
    void setSourceRegistry(QObject *);

    QObject *tabModel() const;
    void setTabModel(QObject *);

    Q_INVOKABLE void exec();

private:
    Homerun::SourceRegistry *m_sourceRegistry;
    TabModel *m_tabModel;
};

#endif /* CONFIGDIALOG_H */
