/*

chooser widget for KDM

Copyright (C) 2002-2003 Oswald Buddenhagen <ossi@kde.org>
based on the chooser (C) 1999 by Harald Hoyer <Harald.Hoyer@RedHat.de>

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

#ifndef KCHOOSER_H
#define KCHOOSER_H

#include "kgdialog.h"

#include <QTimer>

class ChooserListViewItem;

class QLineEdit;
class QSocketNotifier;
class QTreeWidget;

class ChooserDlg : public KGDialog {
    Q_OBJECT
    typedef KGDialog inherited;

  public:
    ChooserDlg();

  public Q_SLOTS:
    void slotReadPipe();
    void addHostname();
//    void slotHelp();
    void pingHosts();
    void accept();
    void reject();

  private Q_SLOTS:
    void slotTimeout();
    void slotActivity();

  private:
    QString recvStr();
    ChooserListViewItem *findItem(int id);

    QTimer timer;
    QTreeWidget *host_view;
    QLineEdit *iline;
    QSocketNotifier *sn;
};

#endif /* KCHOOSER_H */
