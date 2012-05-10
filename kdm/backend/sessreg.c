/*

Copyright 1990, 1998  The Open Group
Copyright 2005 Oswald Buddenhagen <ossi@kde.org>

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/

/*

  Author: Keith Packard, MIT X Consortium
  Lastlog support and dynamic utmp entry allocation
    by Andreas Stolcke <stolcke@icsi.berkeley.edu>

*/

#define _FILE_OFFSET_BITS 64
#include "dm.h"
#include "dm_error.h"

#if defined(__svr4__) || defined(__Lynx__) || defined(__QNX__) || defined(__APPLE__) || defined(_SEQUENT_) /*|| defined(USE_PAM)*/
# define NO_LASTLOG
#endif
#ifdef __FreeBSD__
# if __FreeBSD_version >= 900007
#  define NO_LASTLOG
# endif
#endif

#ifndef NO_LASTLOG
# ifdef HAVE_LASTLOG_H
#  include <lastlog.h>
# endif
# ifndef LLOG_FILE
#  ifdef _PATH_LASTLOGX
#   define LLOG_FILE _PATH_LASTLOGX
#  elif defined(_PATH_LASTLOG)
#   define LLOG_FILE _PATH_LASTLOG
#  else
#   define LLOG_FILE "/usr/adm/lastlog"
#  endif
# endif
#endif

#ifdef BSD
# if !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__)
/* *BSD doesn't like a ':0' type entry in utmp */
#  define NO_UTMP
# endif
#endif

#ifdef BSD_UTMP
# ifndef TTYS_FILE
#  define TTYS_FILE "/etc/ttys"
# endif
#endif

#ifdef _AIX
# define UTL_PFX "xdm/"
# define UTL_OFF strlen(UTL_PFX)
#else
# define UTL_OFF 0
#endif

#ifndef BSD_UTMP
static unsigned
crc32s(const char *str)
{
    int b;
    unsigned crc = 0xffffffff, by;

    for (; *str; str++) {
        by = (crc & 255) ^ *(const unsigned char *)str;
        for (b = 0; b < 8; b++)
            by = (by >> 1) ^ (-(by & 1) & 0xedb88320);
        crc = (crc >> 8) ^ by;
    }
    return crc;
}
#endif

#if (!defined(NO_UTMP) && defined(BSD_UTMP)) || \
    !defined(HAVE_UPDWTMP) || \
    (!defined(NO_LASTLOG) && !defined(HAVE_LASTLOGX))
static void
writeOut(int fd, const void *buf, size_t len, const char *msg)
{
    ssize_t ret;
    if ((ret = write(fd, buf, len)) != (ssize_t)len) {
        if (ret >= 0)
            errno = -ENOSPC;
        logError(msg);
    }
    if (close(fd) < 0)
        logError(msg);
}
#endif

void
sessreg(struct display *d, int pid, const char *user, int uid)
{
    const char *dot, *colon;
    int left, clen, c;
#ifdef BSD_UTMP
    FILE *ttys;
    int utmp, slot, freeslot;
    STRUCTUTMP entry;
#else
    unsigned crc, i;
#endif
#ifndef HAVE_UPDWTMP
    int wtmp;
#endif
#ifndef NO_LASTLOG
# ifdef HAVE_LASTLOGX
    struct lastlogx ll;
#  define ll_time ll_tv.tv_sec
# else
    int llog;
    struct lastlog ll;
# endif
#endif
    STRUCTUTMP ut_ent;

    if (!d->useSessReg)
        return;

    bzero(&ut_ent, sizeof(ut_ent));

    if (pid) {
        strncpy(ut_ent.ut_user, user, sizeof(ut_ent.ut_user));
#ifndef BSD_UTMP
        ut_ent.ut_pid = pid;
        ut_ent.ut_type = USER_PROCESS;
    } else {
        ut_ent.ut_type = DEAD_PROCESS;
#endif
    }
    ut_ent.ut_time = time(0);

    colon = strchr(d->name, ':');
    clen = strlen(colon);
    if (clen > (int)(sizeof(ut_ent.ut_line) - UTL_OFF) - 2)
        return; /* uhm, well ... */
    if (colon == d->name) {
#ifndef BSD_UTMP
        strncpy(ut_ent.ut_id, d->name, sizeof(ut_ent.ut_id));
#endif
        left = 0;
    } else {
#ifdef HAVE_STRUCT_UTMP_UT_HOST
# ifndef BSD_UTMP
        if (pid)
# endif
        {
            if (colon - d->name > (int)sizeof(ut_ent.ut_host)) {
                ut_ent.ut_host[0] = '~';
                memcpy(ut_ent.ut_host + 1,
                       colon - (sizeof(ut_ent.ut_host) - 1),
                       sizeof(ut_ent.ut_host) - 1);
            } else {
                memcpy(ut_ent.ut_host, d->name, colon - d->name);
            }
        }
#endif
#ifndef BSD_UTMP
        crc = crc32s(d->name);
        ut_ent.ut_id[0] = crc % 26 + 'A';
        crc /= 26;
        for (i = 1; i < sizeof(ut_ent.ut_id); i++) {
            c = crc % 62;
            crc /= 62;
            ut_ent.ut_id[i] = c < 26 ? c + 'A' :
                              c < 52 ? c - 26 + 'a' : c - 52 + '0';
        }
#endif
        left = sizeof(ut_ent.ut_line) - UTL_OFF - clen;
        if (colon - d->name <= left) {
            clen += colon - d->name;
            colon = d->name;
            left = 0;
        } else {
            dot = strchr(d->name, '.');
            if (dot && dot - d->name < left) {
                memcpy(ut_ent.ut_line + UTL_OFF, d->name, left - 1);
                ut_ent.ut_line[UTL_OFF + left - 1] = '~';
            } else {
                memcpy(ut_ent.ut_line + UTL_OFF, d->name, left / 2 - 1);
                ut_ent.ut_line[UTL_OFF + left/2 - 1] = '~';
                if (dot) {
                    memcpy(ut_ent.ut_line + UTL_OFF + left / 2,
                           dot - (left - left / 2 - 1),
                           left - left / 2 - 1);
                    ut_ent.ut_line[UTL_OFF + left - 1] = '~';
                } else
                    memcpy(ut_ent.ut_line + UTL_OFF + left / 2,
                           colon - (left - left / 2), left - left / 2);
            }
        }
    }
#ifdef UTL_PFX
    memcpy(ut_ent.ut_line, UTL_PFX, UTL_OFF);
#endif
    memcpy(ut_ent.ut_line + UTL_OFF + left, colon, clen);

#ifndef NO_UTMP
# ifdef BSD_UTMP
    if ((utmp = open(UTMP_FILE, O_RDWR)) < 0) {
        debug("cannot open utmp file " UTMP_FILE ": %m\n");
    } else {

        slot = 1;
        if (pid) {
            if (!(ttys = fopen(TTYS_FILE, "r"))) {
                logWarn("Cannot open tty file " TTYS_FILE ": %m\n");
            } else {
                int column0 = True;
                while ((c = getc(ttys)) != EOF)
                    if (c == '\n') {
                        slot++;
                        column0 = True;
                    } else {
                        column0 = False;
                    }
                if (!column0)
                    slot++;
                fclose(ttys);
            }
        }
        freeslot = -1;
        lseek(utmp, slot * sizeof(entry), SEEK_SET);
        while (read(utmp, (char *)&entry, sizeof(entry)) == sizeof(entry)) {
            if (!strncmp(entry.ut_line, ut_ent.ut_line,
                         sizeof(entry.ut_line)))
#  ifdef HAVE_STRUCT_UTMP_UT_HOST
                if (!strncmp(entry.ut_host, ut_ent.ut_host,
                             sizeof(entry.ut_host)))
#  endif
                    goto found;
            if (freeslot < 0 && *entry.ut_user == '\0')
                freeslot = slot;
            slot++;
        }
        if (!pid) {
            debug("utmp entry for display %s vanished\n", d->name);
            close(utmp);
            goto skip;
        }
        if (freeslot >= 0)
            slot = freeslot;
      found:

#  ifdef HAVE_STRUCT_UTMP_UT_HOST
        if (!pid)
            bzero(ut_ent.ut_host, sizeof(ut_ent.ut_host));
#  endif
        lseek(utmp, slot * sizeof(ut_ent), SEEK_SET);
        writeOut(utmp, &ut_ent, sizeof(ut_ent),
                 "Cannot write utmp file " UTMP_FILE ": %m\n");
    }
  skip:
# else
    SETUTENT();
    PUTUTLINE(&ut_ent); /* Returns void on some systems => no error check. */
    ENDUTENT();
# endif
#endif

#ifdef HAVE_UPDWTMP
# ifdef HAVE_UTMPX
    updwtmpx(WTMP_FILE, &ut_ent);
# else
    updwtmp(WTMP_FILE, &ut_ent);
# endif
#else
    if ((wtmp = open(WTMP_FILE, O_WRONLY | O_APPEND)) < 0)
        debug("cannot open wtmp file " WTMP_FILE ": %m\n");
    else
        writeOut(wtmp, &ut_ent, sizeof(ut_ent),
                 "Cannot write wtmp file " WTMP_FILE ": %m\n");
#endif

#ifndef NO_LASTLOG
    if (pid) {
        bzero((char *)&ll, sizeof(ll));
        ll.ll_time = ut_ent.ut_time;
        memcpy(ll.ll_line, ut_ent.ut_line, sizeof(ll.ll_line));
        memcpy(ll.ll_host, ut_ent.ut_host, sizeof(ll.ll_host));
# ifdef HAVE_LASTLOGX
        updlastlogx(LLOG_FILE, uid, &ll);
# else
        if ((llog = open(LLOG_FILE, O_RDWR)) < 0) {
            debug("cannot open lastlog file " LLOG_FILE ": %m\n");
        } else {
            lseek(llog, (off_t)uid * sizeof(ll), SEEK_SET);
            writeOut(llog, &ll, sizeof(ll),
                     "Cannot write lastlog file " LLOG_FILE ": %m\n");
        }
# endif
    }
#else
    (void)uid;
#endif

#ifdef UTL_PFX
    {
        char tmp[sizeof("/dev/") + sizeof(ut_ent.ut_line)];
        mkdir("/dev/" UTL_PFX, 0755);
        chmod("/dev/" UTL_PFX, 0755);
        sprintf(tmp, "/dev/%.*s", sizeof(ut_ent.ut_line), ut_ent.ut_line);
        if (pid)
            close(creat(tmp, 0644));
        else
            unlink(tmp);
    }
#endif
}
