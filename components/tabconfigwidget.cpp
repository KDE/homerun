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
#include <tabconfigwidget.h>

// Local
#include <tabcontentconfigwidget.h>
#include <tabmodel.h>

// KDE
#include <KIcon>

// Qt
#include <QPersistentModelIndex>

using namespace Homerun;

TabConfigWidget::TabConfigWidget(TabModel *tabModel, SourceRegistry *registry)
: m_tabModel(tabModel)
{
    Q_ASSERT(m_tabModel);
    for (int row = 0; row < m_tabModel->rowCount(); ++row) {
        QModelIndex index = m_tabModel->index(row, 0);
        QString iconName = index.data(Qt::DecorationRole).toString();
        QString text = index.data(Qt::DisplayRole).toString();
        TabContentConfigWidget *widget = new TabContentConfigWidget(index, registry);
        addTab(widget, KIcon(iconName), text);
    }
}

TabConfigWidget::~TabConfigWidget()
{
}

#include <tabconfigwidget.moc>
