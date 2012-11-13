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

// KDE
#include <KDebug>
#include <KPluginInfo>
#include <Plasma/RunnerManager>

// Qt
#include <QListWidgetItem>

Q_DECLARE_METATYPE(KPluginInfo)

static const char *WHITELIST_KEY = "whitelist";

namespace Homerun
{

static QListWidgetItem *createWidgetItem(const KPluginInfo &info)
{
    QListWidgetItem *item = new QListWidgetItem(info.name());
    item->setIcon(KIcon(info.icon()));
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setData(Qt::UserRole, QVariant::fromValue(info));
    return item;
}

RunnerConfigurationWidget::RunnerConfigurationWidget(const KConfigGroup &group)
: SourceConfigurationWidget(group)
{
    setupUi(this);
    m_searchLine->setListWidget(m_listWidget);

    // A runner is enabled if there is either a whitelist and it is part of it
    // or if there is no whitelist and its EnabledByDefault key is true

    QStringList whiteList = group.readEntry(WHITELIST_KEY, QStringList());
    bool hasWhiteList = !whiteList.isEmpty();

    KPluginInfo::List list = Plasma::RunnerManager::listRunnerInfo();
    Q_FOREACH(const KPluginInfo &info, list) {
        QListWidgetItem *item = createWidgetItem(info);
        bool selected;
        if (hasWhiteList) {
            selected = whiteList.contains(info.pluginName());
        } else {
            selected = info.isPluginEnabledByDefault();
        }
        item->setCheckState(selected ? Qt::Checked : Qt::Unchecked);
        m_listWidget->addItem(item);
    }
    m_listWidget->sortItems();
}

void RunnerConfigurationWidget::save()
{
    QStringList whiteList;
    bool hasChanges = false;

    // Only write a whiteList if the selected runner list is not the same as
    // the list of all EnabledByDefault runners

    for (int idx = 0; idx < m_listWidget->count(); ++idx) {
        QListWidgetItem *item = m_listWidget->item(idx);
        KPluginInfo info = item->data(Qt::UserRole).value<KPluginInfo>();

        bool selected = item->checkState() == Qt::Checked;
        if (selected != info.isPluginEnabledByDefault()) {
            hasChanges = true;
        }
        if (selected) {
            whiteList << info.pluginName();
        }
    }
    if (hasChanges) {
        configGroup().writeEntry(WHITELIST_KEY, whiteList);
    } else {
        configGroup().deleteEntry(WHITELIST_KEY);
    }
}

} // namespace Homerun

#include <runnerconfigurationwidget.moc>
