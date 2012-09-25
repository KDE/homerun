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
#include <tabcontentconfigwidget.h>

// Local
#include <tabmodel.h>
#include <ui_tabcontentconfigwidget.h>

// libhomerun
#include <abstractsource.h>
#include <sourceid.h>
#include <sourceregistry.h>

// KDE
#include <KDebug>

// Qt
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

using namespace Homerun;

class AvailableSourcesProxyModel : public QSortFilterProxyModel
{
public:
    AvailableSourcesProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    {}

    Qt::ItemFlags flags(const QModelIndex &index) const
    {
        return QSortFilterProxyModel::flags(index) | Qt::ItemIsDragEnabled;
    }
};

TabContentConfigWidget::TabContentConfigWidget(const QModelIndex &index, SourceRegistry *registry)
: m_index(index)
, m_sourceRegistry(registry)
, m_ui(new Ui_TabContentConfigWidget)
, m_availableSourcesModel(new AvailableSourcesProxyModel(this))
, m_selectedSourcesModel(new QStandardItemModel(this))
{
    m_ui->setupUi(this);

    m_availableSourcesModel->setSourceModel(m_sourceRegistry->availableSourcesModel());

    fillSelectedSourcesModel();

    m_ui->availableListView->setModel(m_availableSourcesModel);
    m_ui->selectedListView->setModel(m_selectedSourcesModel);

    connect(m_selectedSourcesModel, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(updateTabModel()));
    connect(m_selectedSourcesModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(updateTabModel()));
    connect(m_selectedSourcesModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), SLOT(updateTabModel()));
}

TabContentConfigWidget::~TabContentConfigWidget()
{
    delete m_ui;
}

void TabContentConfigWidget::fillSelectedSourcesModel()
{
    m_selectedSourcesModel->clear();
    QStringList lst = m_index.data(TabModel::SourcesRole).toStringList();
    Q_FOREACH(const QString &idString, lst) {
        bool ok;
        SourceId id = SourceId::fromString(idString, &ok);
        if (!ok) {
            kWarning() << "Could not parse" << idString;
            continue;
        }
        AbstractSource *source = m_sourceRegistry->sourceByName(id.name());
        QStandardItem *item = new QStandardItem(id.name());
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
        m_selectedSourcesModel->appendRow(item);
    }
}

void TabContentConfigWidget::updateTabModel()
{
    QStringList lst;
    for (int row = 0; row < m_selectedSourcesModel->rowCount(); ++row) {
        QModelIndex index = m_selectedSourcesModel->index(row, 0);
        lst << index.data(Qt::DisplayRole).toString();
    }
    QAbstractItemModel *model = const_cast<QAbstractItemModel *>(m_index.model());
    model->setData(m_index, lst, TabModel::SourcesRole);
}

#include <tabcontentconfigwidget.moc>
