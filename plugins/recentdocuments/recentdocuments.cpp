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
#include <recentdocuments.h>

// Homerun
#include <abstractsource.h>
#include <sourceregistry.h>

// KDE
#include <KDebug>
#include <KDesktopFile>
#include <KDirWatch>
#include <KLocale>
#include <KPluginFactory>
#include <KRecentDocument>
#include <KRun>

typedef Homerun::SimpleSource<RecentDocumentsModel> RecentDocumentsSource;
HOMERUN_EXPORT_SOURCE(recentdocuments, RecentDocumentsSource)

RecentDocumentsModel::RecentDocumentsModel()
{
    KDirWatch *watch = new KDirWatch(this);
    watch->addDir(KRecentDocument::recentDocumentDirectory());

    connect(watch, SIGNAL(created(QString)), SLOT(load()));
    connect(watch, SIGNAL(deleted(QString)), SLOT(load()));
    connect(watch, SIGNAL(dirty(QString)), SLOT(load()));

    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::DecorationRole, "icon");
    setRoleNames(roles);

    load();
}

void RecentDocumentsModel::load()
{
    clear();
    QSet<QString> urls;
    Q_FOREACH(const QString &path, KRecentDocument::recentDocuments()) {
        KDesktopFile file(path);
        QString url = file.readUrl();

        // Prevent duplicate entries
        if (urls.contains(url)) {
            continue;
        }
        urls.insert(url);

        QStandardItem *item = new QStandardItem(file.readName());
        item->setData(file.readIcon(), Qt::DecorationRole);
        item->setData(url, Qt::UserRole);
        appendRow(item);
    }
    countChanged();
}

bool RecentDocumentsModel::trigger(int row)
{
    QStandardItem *itm = item(row);
    Q_ASSERT(itm);
    KUrl url = itm->data(Qt::UserRole).toString();
    new KRun(url, 0);
    return true;
}

QString RecentDocumentsModel::name() const
{
    return i18n("Recent Documents");
}

int RecentDocumentsModel::count() const
{
    return rowCount(QModelIndex());
}

#include <recentdocuments.moc>
