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
{
    // Init model
    QStandardItem *defaultItem = fillModel(group.readEntry("entryPath"));
    m_model->setSortRole(SortRole);
    m_model->sort(0);

    m_ui->setupUi(this);
    m_ui->treeView->setModel(m_model);

    // Select currently configured item
    Q_ASSERT(defaultItem);
    QModelIndex index = defaultItem->index();
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


// Helper struct for fillModel
struct ItemCreator
{
    ItemCreator()
    : m_item(0)
    {}
    QString m_entryPath;
    QStandardItem *m_item;

    void createItemChildren(QStandardItem *parent, KServiceGroup::Ptr parentGroup)
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

            if (m_entryPath == entryPath) {
                m_item = item;
            }
        }
    }
};

QStandardItem *InstalledAppsConfigurationWidget::fillModel(const QString &entryPath)
{
    KServiceGroup::Ptr group = KServiceGroup::root();
    QStandardItem *rootItem = new QStandardItem;
    rootItem->setText(i18n("All Applications"));
    rootItem->setData(QString(), EntryPathRole);
    m_model->appendRow(rootItem);

    ItemCreator creator;
    creator.m_entryPath = entryPath;
    creator.createItemChildren(rootItem, group);
    return creator.m_item ? creator.m_item : rootItem;
}

} // namespace Homerun

#include <installedappsconfigurationwidget.moc>
