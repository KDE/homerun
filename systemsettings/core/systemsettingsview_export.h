/**************************************************************************
 * Copyright (C) 2009 Ben Cooksley <bcooksley@kde.org>                    *
 *                                                                        *
 * This program is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU General Public License            *
 * as published by the Free Software Foundation; either version 2         *
 * of the License, or (at your option) any later version.                 *
 *                                                                        *
 * This program is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU General Public License for more details.                           *
 *                                                                        *
 * You should have received a copy of the GNU General Public License      *
 * along with this program; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA          *
 * 02110-1301, USA.                                                       *
***************************************************************************/

#ifndef SYSTEMSETTINGSVIEW_EXPORT_H
#define SYSTEMSETTINGSVIEW_EXPORT_H
 
// needed for KDE_EXPORT and KDE_IMPORT macros
#include <kdemacros.h>
 
#ifndef SYSTEMSETTINGSVIEW_EXPORT
# if defined(MAKE_SYSTEMSETTINGSVIEW_LIB)
   // We are building this library
#  define SYSTEMSETTINGSVIEW_EXPORT KDE_EXPORT
# else
   // We are using this library
#  define SYSTEMSETTINGSVIEW_EXPORT KDE_IMPORT
# endif
#endif
 
# ifndef SYSTEMSETTINGSVIEW_EXPORT_DEPRECATED
#  define SYSTEMSETTINGSVIEW_EXPORT_DEPRECATED KDE_DEPRECATED SYSTEMSETTINGSVIEW_EXPORT
# endif
 
#endif
