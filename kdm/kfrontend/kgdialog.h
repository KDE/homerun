/*

Base class for various kdm greeter dialogs

Copyright (C) 1997, 1998 Steffen Hansen <hansen@kde.org>
Copyright (C) 2000-2004 Oswald Buddenhagen <ossi@kde.org>


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


#ifndef KGDIALOG_H
#define KGDIALOG_H

#include <config-workspace.h>
#include <config-kdm.h> // for WITH_KDM_XCONSOLE

#include "kdmconfig.h"
#include "kfdialog.h"

class KConsole;
class KGVerify;

class QMenu;

#define ex_exit    1
#define ex_greet   2
#define ex_choose  3
#define ex_login   4

class KGDialog : public FDialog {
    Q_OBJECT
    typedef FDialog inherited;

  public:
    KGDialog(bool themed = false);

  public Q_SLOTS:
    void slotActivateMenu(QAction *);
    void slotActivateMenu(bool);
    void slotExit();
    void slotSwitch();
    void slotConsole();
    void slotShutdown();

  protected:
#ifdef XDMCP
    void completeMenu(int _switchIf, int _switchCode, const QString &_switchMsg, int _switchAccel);
#else
    void completeMenu();
#endif
    void inserten(const QKeySequence &shortcut, const char *member, int data = -1);
    void inserten(const QString &txt, const QKeySequence &shortcut, const char *member);
    void inserten(const QString &txt, const QKeySequence &shortcut, QMenu *cmnu);

    bool needSep;
    QMenu *optMenu;
    KGVerify *verify;
#ifdef WITH_KDM_XCONSOLE
    KConsole *consoleView;
#endif

  private Q_SLOTS:
    void slotDisplaySelected(QAction *);
    void slotPopulateDisplays();

  private:
    void ensureMenu();

#ifdef HAVE_VTS
    QMenu *dpyMenu;
#endif
    int switchCode;
};

#endif /* KGDIALOG_H */
