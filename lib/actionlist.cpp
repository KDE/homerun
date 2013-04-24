/*
Copyright 2013 Aurélien Gâteau <agateau@kde.org>

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
#include <actionlist.h>

// Local

// KDE
#include <KDebug>
#include <KIcon>
#include <KLocale>
#include <KMimeTypeTrader>
#include <KPropertiesDialog>
#include <KRun>
#include <KService>

// libkonq
#include <konq_operations.h>

// Qt
#include <QApplication>

namespace Homerun
{

namespace ActionList
{

QVariantMap createActionItem(const QString &label, const QString &actionId, const QVariant &actionArg)
{
    QVariantMap map;
    map["text"] = label;
    map["actionId"] = actionId;
    if (actionArg.isValid()) {
        map["actionArgument"] = actionArg;
    }
    return map;
}

QVariantMap createTitleActionItem(const QString &label)
{
    QVariantMap map;
    map["text"] = label;
    map["type"] = "title";
    return map;
}

QVariantMap createSeparatorActionItem()
{
    QVariantMap map;
    map["type"] = "separator";
    return map;
}

static QVariantMap createEmptyTrashItem()
{
    QVariantMap map = createActionItem(
        i18nc("@action:inmenu", "Empty Trash"),
        "_homerun_fileItem_emptyTrash");
    map["icon"] = KIcon("trash-empty");

    KConfig trashConfig("trashrc", KConfig::SimpleConfig);
    const bool isEmpty = trashConfig.group("Status").readEntry("Empty", true);
    map["enabled"] = !isEmpty;
    return map;
}

QVariantList createListForFileItem(const KFileItem &fileItem)
{
    QVariantList list;
    if (fileItem.url() == KUrl("trash:/")) {
        list << createEmptyTrashItem() << createSeparatorActionItem();
    }
    KService::List services = KMimeTypeTrader::self()->query(fileItem.mimetype(), "Application");
    if (!services.isEmpty()) {
        list << createTitleActionItem(i18n("Open with:"));
        Q_FOREACH(const KService::Ptr service, services) {
            const QString text = service->name().replace('&', "&&");
            QVariantMap item = createActionItem(text, "_homerun_fileItem_openWith", service->entryPath());
            QString iconName = service->icon();
            if (!iconName.isEmpty()) {
                item["icon"] = KIcon(service->icon());
            }
            list << item;
        }
        list << createSeparatorActionItem();
    }
    list << createActionItem(i18n("Properties"), "_homerun_fileItem_properties");
    return list;
}

bool handleFileItemAction(const KFileItem &fileItem, const QString &actionId, const QVariant &actionArg, bool *close)
{
    if (actionId == "_homerun_fileItem_properties") {
        KPropertiesDialog *dlg = new KPropertiesDialog(fileItem, QApplication::activeWindow());
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->show();
        *close = false;
        return true;
    }

    if (actionId == "_homerun_fileItem_openWith") {
        const QString path = actionArg.toString();
        const KService::Ptr service = KService::serviceByDesktopPath(path);
        if (!service) {
            kWarning() << "Invalid service";
            return false;
        }
        KRun::run(*service, KUrl::List() << fileItem.url(), QApplication::activeWindow());
        *close = true;
        return true;
    }

    if (actionId == "_homerun_fileItem_emptyTrash") {
        KonqOperations::emptyTrash(QApplication::activeWindow());
        return true;
    }

    return false;
}

} // namespace ActionList
} // namespace Homerun
