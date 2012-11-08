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
#include <installedappsconfigurationwidget.h>

// Local
#include <ui_installedappsconfigurationwidget.h>

// KDE
#include <KDebug>
#include <KIcon>
#include <KServiceGroup>
#include <KSycocaEntry>

// Qt
#include <QStandardItem>
#include <QStandardItemModel>

enum {
    EntryPathRole = Qt::UserRole + 1,
    SortRole,
};

namespace Homerun
{

InstalledAppsConfigurationWidget::InstalledAppsConfigurationWidget(const KConfigGroup &group)
: SourceConfigurationWidget(group)
, m_ui(new Ui_InstalledAppsConfigurationWidget)
, m_model(new QStandardItemModel(this))
, m_defaultItem(0)
{
    m_defaultEntryPath = group.readEntry("entryPath");

    // Init model
    fillModel();
    m_model->setSortRole(SortRole);
    m_model->sort(0);

    m_ui->setupUi(this);
    m_ui->treeView->setModel(m_model);

    // Select currently configured item
    Q_ASSERT(m_defaultItem);
    QModelIndex index = m_defaultItem->index();
    m_ui->treeView->selectionModel()->select(index, QItemSelectionModel::Select);

    // Expand up to the configured item
    index = index.parent();
    for (; index.isValid(); index = index.parent()) {
        m_ui->treeView->setExpanded(index, true);
    }

    // Always expand top level
    m_ui->treeView->setExpanded(m_model->index(0, 0), true);
}

InstalledAppsConfigurationWidget::~InstalledAppsConfigurationWidget()
{
    delete m_ui;
}

void InstalledAppsConfigurationWidget::save()
{
    QModelIndex index = m_ui->treeView->selectionModel()->currentIndex();
    if (!index.isValid()) {
        kWarning() << "No item selected!";
        return;
    }
    QString entryPath = index.data(EntryPathRole).toString();
    configGroup().writeEntry("entryPath", entryPath);
}

void InstalledAppsConfigurationWidget::fillModel()
{
    KServiceGroup::Ptr group = KServiceGroup::root();
    QStandardItem *item = new QStandardItem;
    item->setText(i18n("All Applications"));
    item->setData(QString(), EntryPathRole);
    m_model->appendRow(item);
    m_defaultItem = item;

    createItemChildren(item, group);
}

void InstalledAppsConfigurationWidget::createItemChildren(QStandardItem *parent, KServiceGroup::Ptr parentGroup)
{
    KServiceGroup::List list = parentGroup->entries(false /* sorted: set to false as it does not seem to work */);

    Q_FOREACH(const KSycocaEntry::Ptr entry, list) {
        if (!entry->isType(KST_KServiceGroup)) {
            continue;
        }
        KServiceGroup::Ptr group = KServiceGroup::Ptr::staticCast(entry);

        if (group->noDisplay() || group->childCount() == 0) {
            continue;
        }
        QString entryPath = group->entryPath();

        QStandardItem *item = new QStandardItem;
        item->setText(group->caption());
        item->setIcon(KIcon(group->icon()));
        item->setData(entryPath, EntryPathRole);
        item->setData(group->caption().toLower(), SortRole);

        parent->appendRow(item);
        createItemChildren(item, group);

        if (m_defaultEntryPath == entryPath) {
            m_defaultItem = item;
        }
    }
}

} // namespace Homerun

#include <installedappsconfigurationwidget.moc>
