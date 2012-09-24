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
#include <configdialog.h>

// Local
#include <tabmodel.h>
#include <tabconfigwidget.h>

// libhomerun
#include <sourceregistry.h>

// KDE
#include <KDebug>
#include <KDialog>

// Qt
#include <QApplication>
#include <QScopedPointer>

using namespace Homerun;

ConfigDialog::ConfigDialog(QObject *parent)
: QObject(parent)
, m_sourceRegistry(0)
, m_tabModel(0)
{
}

ConfigDialog::~ConfigDialog()
{
}

QObject *ConfigDialog::sourceRegistry() const
{
    return m_sourceRegistry;
}

void ConfigDialog::setSourceRegistry(QObject *obj)
{
    if (!obj) {
        m_sourceRegistry = 0;
        return;
    }
    SourceRegistry *registry = qobject_cast<SourceRegistry *>(obj);
    Q_ASSERT(registry);
    m_sourceRegistry = registry;
}

QObject *ConfigDialog::tabModel() const
{
    return m_tabModel;
}

void ConfigDialog::setTabModel(QObject *obj)
{
    if (!obj) {
        m_tabModel = 0;
        return;
    }
    TabModel *tabModel = qobject_cast<TabModel *>(obj);
    Q_ASSERT(tabModel);
    m_tabModel = tabModel;
}

void ConfigDialog::exec()
{
    Q_ASSERT(m_tabModel);
    QScopedPointer<KDialog> dialog(new KDialog(QApplication::activeWindow()));

    TabConfigWidget *widget = new TabConfigWidget(m_tabModel, m_sourceRegistry);
    dialog->setMainWidget(widget);

    dialog->exec();
}

#include <configdialog.moc>
