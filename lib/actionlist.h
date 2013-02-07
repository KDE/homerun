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
#ifndef ACTIONLIST_H
#define ACTIONLIST_H

// Local
#include <homerun_export.h>

// Qt
#include <QVariant>

// KDE

class KFileItem;

namespace Homerun
{

/**
 * Helper functions to create action list items
 */
namespace ActionList
{

QVariantMap HOMERUN_EXPORT createActionItem(const QString &label, const QString &actionId, const QVariant &actionArg = QVariant());

QVariantMap HOMERUN_EXPORT createTitleActionItem(const QString &label);

QVariantMap HOMERUN_EXPORT createSeparatorActionItem();

QVariantList HOMERUN_EXPORT createListForFileItem(const KFileItem &fileItem);

/**
 * Handle action if it has been created by this namespace
 * @param fileItem The file item for which the action was created
 * @param actionId The action id
 * @param actionArg The action argument
 * @param close Set to true if homerun should close. Only makes sense if the action has been handled. Cannot be NULL.
 * @return true if the action has been handled
 */
bool HOMERUN_EXPORT handleFileItemAction(const KFileItem &fileItem, const QString &actionId, const QVariant &actionArg, bool *close);

} // namespace ActionList
} // namespace Homerun

#endif /* ACTIONLIST_H */
