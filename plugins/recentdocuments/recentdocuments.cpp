/*
Copyright 2012 Aurélien Gâteau <agateau@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
// Self
#include <recentdocuments.h>

// Homerun
#include <abstractsource.h>
#include <actionlist.h>

// KDE
#include <KDebug>
#include <KDesktopFile>
#include <KDirWatch>
#include <KFileItem>
#include <KLocale>
#include <KPluginFactory>
#include <KRecentDocument>
#include <KRun>

// Qt
#include <QFile>

typedef Homerun::SimpleSource<RecentDocumentsModel> RecentDocumentsSource;
HOMERUN_EXPORT_SOURCE(recentdocuments, RecentDocumentsSource)

RecentDocumentsModel::RecentDocumentsModel()
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::DecorationRole, "decoration");
    roles.insert(HasActionListRole, "hasActionList");
    roles.insert(ActionListRole, "actionList");
    setRoleNames(roles);

    KDirWatch *watch = new KDirWatch(this);
    watch->addDir(KRecentDocument::recentDocumentDirectory());

    connect(watch, SIGNAL(created(QString)), SLOT(load()));
    connect(watch, SIGNAL(deleted(QString)), SLOT(load()));
    connect(watch, SIGNAL(dirty(QString)), SLOT(load()));
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

        QString name = file.readName();
        if (name.isEmpty()) {
            name = url;
        }
        if (name.isEmpty()) {
            kWarning() << "Skipping" << path << ": it has no name or url.";
            continue;
        }
        QStandardItem *item = new QStandardItem(name);
        item->setData(file.readIcon(), Qt::DecorationRole);
        item->setData(path, DesktopPathRole);
        item->setData(url, UrlRole);
        item->setData(true, HasActionListRole);
        appendRow(item);

        urls.insert(url);
    }
    countChanged();
}

bool RecentDocumentsModel::trigger(int row, const QString &actionId, const QVariant &actionArgument)
{
    QStandardItem *itm = item(row);
    Q_ASSERT(itm);
    KUrl url = itm->data(UrlRole).toString();
    if (actionId.isEmpty()) {
        new KRun(url, 0);
        return true;
    }
    if (actionId == "forget") {
        forget(itm);
        return false;
    }
    bool close = false;
    KFileItem item(KFileItem::Unknown, KFileItem::Unknown, url);
    if (Homerun::ActionList::handleFileItemAction(item, actionId, actionArgument, &close)) {
        return close;
    }
    return false;
}

void RecentDocumentsModel::forget(QStandardItem *itm)
{
    Q_ASSERT(itm);
    QString path = itm->data(DesktopPathRole).toString();

    bool ok = QFile::remove(path);
    if (!ok) {
        kWarning() << "Failed to remove" << path;
        return;
    }
    delete itm;
}

QString RecentDocumentsModel::name() const
{
    return i18n("Recent Documents");
}

int RecentDocumentsModel::count() const
{
    return rowCount(QModelIndex());
}

QVariant RecentDocumentsModel::data(const QModelIndex& index, int role) const
{
    if (role != ActionListRole) {
        return QStandardItemModel::data(index, role);
    }
    QStandardItem *itm = itemFromIndex(index);
    if (!itm) {
        return QVariant();
    }
    KUrl url = itm->data(UrlRole).toString();
    KFileItem item(KFileItem::Unknown, KFileItem::Unknown, url);
    QVariantList actionList = Homerun::ActionList::createListForFileItem(item);

    actionList.prepend(Homerun::ActionList::createSeparatorActionItem());
    QVariantMap forgetAction = Homerun::ActionList::createActionItem(i18n("Forget Document"), "forget");
    actionList.prepend(forgetAction);

    return actionList;
}

#include <recentdocuments.moc>
