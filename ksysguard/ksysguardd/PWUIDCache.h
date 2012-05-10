/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2000 Chris Schlaeger <cs@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <sys/types.h>

#ifndef KSG_PWUIDCACHE_H
#define KSG_PWUIDCACHE_H

/**
  getpwuid() can be fairly expensive on NIS or LDAP systems that do not
  use chaching. This module implements a cache for uid to user name
  mappings.
 */
void initPWUIDCache( void );
void exitPWUIDCache( void );

const char* getCachedPWUID( uid_t uid );

#endif
