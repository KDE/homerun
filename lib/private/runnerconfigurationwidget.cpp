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
#include <runnerconfigurationwidget.h>

// Local
#include <sourceid.h>

// KDE
#include <KPluginInfo>
#include <KServiceTypeTrader>

// Qt
#include <QListWidgetItem>

namespace Homerun
{

static QListWidgetItem *createWidgetItem(const KPluginInfo &info)
{
    QListWidgetItem *item = new QListWidgetItem(info.name());
    item->setIcon(KIcon(info.icon()));
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setData(Qt::UserRole, info.pluginName());
    return item;
}

RunnerConfigurationWidget::RunnerConfigurationWidget(const KConfigGroup &group)
: SourceConfigurationWidget(group)
{
    setupUi(this);
    m_searchLine->setListWidget(m_listWidget);

    QStringList selectedRunners = group.readEntry("whitelist", QStringList());

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Runner");
    Q_FOREACH(const KService::Ptr &service, offers) {
        KPluginInfo info(service);
        QListWidgetItem *item = createWidgetItem(info);
        bool selected = selectedRunners.contains(info.pluginName());
        item->setCheckState(selected ? Qt::Checked : Qt::Unchecked);
        m_listWidget->addItem(item);
    }
    m_listWidget->sortItems();
}

void RunnerConfigurationWidget::save()
{
    QStringList runners;

    for (int idx = 0; idx < m_listWidget->count(); ++idx) {
        QListWidgetItem *item = m_listWidget->item(idx);
        if (item->checkState() == Qt::Checked) {
            runners << item->data(Qt::UserRole).toString();
        }
    }
    configGroup().writeEntry("whitelist", runners);
}

} // namespace Homerun

#include <runnerconfigurationwidget.moc>
