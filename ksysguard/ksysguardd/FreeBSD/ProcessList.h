/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2011 David Naylor <naylor.b.david@gmail.com>
    Copyright (c) 1999 Chris Schlaeger <cs@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef _process_list_h_
#define _process_list_h_

struct SensorModul;

void initProcessList(struct SensorModul *);
void exitProcessList(void);

int updateProcessList(void);

void printProcessList(const char *);
void printProcessListInfo(const char *);
void printThreadList(const char *);
void printThreadListInfo(const char *);

void printProcessCount(const char *);
void printProcessCountInfo(const char *);
void printProcessxCount(const char *);
void printProcessxCountInfo(const char *);

void printProcSpawn(const char *);
void printProcSpawnInfo(const char *);

void printLastPID(const char *);
void printLastPIDInfo(const char *);

void killProcess(const char* cmd);
void setPriority(const char* cmd);

#endif
