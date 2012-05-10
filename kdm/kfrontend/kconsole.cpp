/*

xconsole widget for KDM

Copyright (C) 2002-2003 Oswald Buddenhagen <ossi@kde.org>


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

#include "kconsole.h"
#include "kdmconfig.h"
#include "kdm_greet.h"

#include <klocale.h>
#include <kpty.h>

#include <QSocketNotifier>
#include <QScrollBar>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#ifdef HAVE_TERMIOS_H
/* for HP-UX (some versions) the extern C is needed, and for other
   platforms it doesn't hurt */
extern "C" {
#include <termios.h>
}
#endif
#if !defined(__osf__)
#ifdef HAVE_TERMIO_H
/* needed at least on AIX */
#include <termio.h>
#endif
#endif

#if defined (_HPUX_SOURCE)
#define _TERMIOS_INCLUDED
#include <bsdtty.h>
#endif

KConsole::KConsole(QWidget *_parent)
    : inherited(_parent)
    , pty(0)
    , notifier(0)
    , fd(-1)
{
    setReadOnly(true);
    setLineWrapMode(NoWrap);

    if (!openConsole())
        append(i18n("*** Cannot connect to console log ***"));
}

KConsole::~KConsole()
{
    closeConsole();
}

int
KConsole::openConsole()
{
#ifdef TIOCCONS
    static const char on = 1;
#endif

    if (*_logSource) {
        if ((fd = open(_logSource, O_RDONLY | O_NONBLOCK)) >= 0)
            goto gotcon;
        logError("Cannot open log source %s, "
                 "falling back to /dev/console.\n", _logSource);
    }

    pty = new KPty;
    if (!pty->open()) {
        delete pty;
        pty = 0;
        return 0;
    }

#ifdef TIOCCONS
    if (ioctl(pty->slaveFd(), TIOCCONS, &on) < 0) {
        perror("ioctl TIOCCONS");
        delete pty;
        pty = 0;
        return 0;
    }
#else
    int consfd;
    if ((consfd = open("/dev/console", O_RDONLY)) < 0) {
        perror("opening /dev/console");
        delete pty;
        pty = 0;
        return 0;
    }
    if (ioctl(consfd, SRIOCSREDIR, pty->slaveFd()) < 0) {
        perror("ioctl SRIOCSREDIR");
        ::close(consfd);
        delete pty;
        pty = 0;
        return 0;
    }
    ::close(consfd);
#endif
    fd = pty->masterFd();

  gotcon:
    notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(notifier, SIGNAL(activated(int)), SLOT(slotData()));
    return 1;
}

void
KConsole::closeConsole()
{
    delete notifier;
    notifier = 0;
    if (pty) {
        delete pty;
        pty = 0;
    } else {
        ::close(fd);
    }
    fd = -1;
}

void
KConsole::slotData()
{
    int n;
    char buffer[1024];

    if ((n = read(fd, buffer, sizeof(buffer))) <= 0) {
        closeConsole();
        if (n || !openConsole())
            append(i18n("\n*** Lost connection with console log ***"));
    } else {
        QString str(QString::fromLocal8Bit(buffer, n).remove('\r'));
        int pos, opos;
        for (opos = 0; (pos = str.indexOf('\n', opos)) >= 0; opos = pos + 1) {
            if (document()->blockCount() == 100) {
                QTextCursor cur(document());
                cur.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
                cur.removeSelectedText();
            }
            if (!leftover.isEmpty()) {
                append(leftover + str.mid(opos, pos - opos));
                leftover.clear();
            } else {
                append(str.mid(opos, pos - opos));
            }
        }
        leftover += str.mid(opos);
    }
}

#include "kconsole.moc"
