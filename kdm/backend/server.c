/*

Copyright 1988, 1998  The Open Group
Copyright 2001,2003,2005 Oswald Buddenhagen <ossi@kde.org>

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
IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of a copyright holder shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the copyright holder.

*/

/*
 * xdm - display manager daemon
 * Author: Keith Packard, MIT X Consortium
 *
 * server.c - manage the X server
 */

#include "dm.h"
#include "dm_error.h"
#include "dm_socket.h"

#include <X11/Xlib.h>

#include <stdio.h>
#include <signal.h>


struct display *startingServer;
time_t serverTimeout = TO_INF;

char **
prepareServerArgv(struct display *d, const char *args)
{
    char **argv;
#ifdef HAVE_VTS
    char vtstr[8];
#endif

    if (!(argv = parseArgs(0, d->serverCmd)) ||
        !(argv = addStrArr(argv, d->name, -1)))
        exit(47);
#ifdef HAVE_VTS
    if (d->serverVT &&
        !(argv = addStrArr(argv, vtstr,
                           sprintf(vtstr, "vt%d", d->serverVT))))
        exit(47);
#endif
    if (!(argv = parseArgs(argv, args)))
        exit(47);

    if (!changeUser(d->serverUID, d->authFile))
        exit(47);

    return argv;
}

static void
startServerOnce(void)
{
    struct display *d = startingServer;
    char **argv;

    debug("startServerOnce for %s, try %d\n", d->name, ++d->startTries);
    d->serverStatus = starting;
    switch (Fork(&d->serverPid)) {
    case 0:
        argv = prepareServerArgv(d, d->serverArgsLocal);
        if (d->authFile) {
            if (!(argv = addStrArr(argv, "-auth", 5)) ||
                !(argv = addStrArr(argv, d->authFile, -1)))
                exit(47);
        }
        debug("exec %\"[s\n", argv);
        /*
         * give the server SIGUSR1 ignored,
         * it will notice that and send SIGUSR1
         * when ready
         */
        (void)Signal(SIGUSR1, SIG_IGN);
        (void)execv(argv[0], argv);
        logError("X server %\"s cannot be executed\n", argv[0]);
        exit(47);
    case -1:
        logError("X server fork failed\n");
        startServerFailed();
        break;
    default:
        debug("X server forked, pid %d\n", d->serverPid);
        serverTimeout = d->serverTimeout + now;
        break;
    }
}

void
startServer(struct display *d)
{
    startingServer = d;
    d->startTries = 0;
    startServerOnce();
}

void
abortStartServer(struct display *d)
{
    if (startingServer == d) {
        if (d->serverStatus != ignore) {
            d->serverStatus = ignore;
            serverTimeout = TO_INF;
            debug("aborting X server start\n");
        }
        startingServer = 0;
    }
}

void
startServerSuccess()
{
    struct display *d = startingServer;
    d->serverStatus = ignore;
    serverTimeout = TO_INF;
    debug("X server ready, starting session\n");
    startDisplayP2(d);
}

void
startServerFailed()
{
    struct display *d = startingServer;
    if (!d->serverAttempts || d->startTries < d->serverAttempts) {
        d->serverStatus = pausing;
        serverTimeout = d->openDelay + now;
    } else {
        d->serverStatus = ignore;
        serverTimeout = TO_INF;
        startingServer = 0;
        logError("X server for display %s cannot be started,"
                 " session disabled\n", d->name);
        stopDisplay(d);
    }
}

void
startServerTimeout()
{
    struct display *d = startingServer;
    switch (d->serverStatus) {
    case ignore:
    case awaiting:
        break; /* cannot happen */
    case starting:
        logError("X server startup timeout, terminating\n");
        kill(d->serverPid, SIGTERM);
        d->serverStatus = terminated;
        serverTimeout = d->serverTimeout + now;
        break;
    case terminated:
        logInfo("X server termination timeout, killing\n");
        kill(d->serverPid, SIGKILL);
        d->serverStatus = killed;
        serverTimeout = 10 + now;
        break;
    case killed:
        logInfo("X server is stuck in D state; leaving it alone\n");
        startServerFailed();
        break;
    case pausing:
        startServerOnce();
        break;
    }
}


Display *dpy;

/*
 * this code is complicated by some TCP failings.  On
 * many systems, the connect will occasionally hang forever,
 * this trouble is avoided by setting up a timeout to Longjmp
 * out of the connect (possibly leaving piles of garbage around
 * inside Xlib) and give up, terminating the server.
 */

static Jmp_buf openAbort;

/* ARGSUSED */
static void
abortOpen(int n ATTR_UNUSED)
{
    Longjmp(openAbort, 1);
}

#ifdef XDMCP

#ifdef STREAMSCONN
# include <tiuser.h>
#endif

static void
getRemoteAddress(struct display *d, int fd)
{
    char buf[512];
    int len = sizeof(buf);
#ifdef STREAMSCONN
    struct netbuf netb;
#endif

    XdmcpDisposeARRAY8(&d->peer);
#ifdef STREAMSCONN
    netb.maxlen = sizeof(buf);
    netb.buf = buf;
    t_getname(fd, &netb, REMOTENAME);
    len = 8;
    /* lucky for us, t_getname returns something that looks like a sockaddr */
#else
    getpeername(fd, (struct sockaddr *)buf, (void *)&len);
#endif
    if (len && XdmcpAllocARRAY8(&d->peer, len))
        memmove(d->peer.data, buf, len);
    debug("got remote address %s %d\n", d->name, d->peer.length);
}

#endif /* XDMCP */

static int
openErrorHandler(Display *dspl)
{
    logError("IO Error in XOpenDisplay(%s)\n", DisplayString(dspl));
    exit(EX_OPENFAILED_DPY);
    /*NOTREACHED*/
    return (0);
}

void
waitForServer(struct display *d)
{
    volatile int i;
    /* static int i; */

    i = 0;
    do {
        (void)Signal(SIGALRM, abortOpen);
        (void)alarm((unsigned)d->openTimeout);
        if (!Setjmp(openAbort)) {
            debug("before XOpenDisplay(%s)\n", d->name);
            errno = 0;
            (void)XSetIOErrorHandler(openErrorHandler);
            dpy = XOpenDisplay(d->name);
#ifdef STREAMSCONN
            {
                /* For some reason, the next XOpenDisplay we do is
                   going to fail, so we might as well get that out
                   of the way.  There is something broken here. */
                Display *bogusDpy = XOpenDisplay(d->name);
                debug("bogus XOpenDisplay %s\n",
                      bogusDpy ? "succeeded" : "failed");
                if (bogusDpy)
                    XCloseDisplay(bogusDpy);    /* just in case */
            }
#endif
            (void)alarm((unsigned)0);
            (void)Signal(SIGALRM, SIG_DFL);
            (void)XSetIOErrorHandler((int (*)(Display *)) 0);
            debug("after XOpenDisplay(%s)\n", d->name);
            if (dpy) {
#ifdef XDMCP
                if ((d->displayType & d_location) == dForeign)
                    getRemoteAddress(d, ConnectionNumber(dpy));
#endif
                registerCloseOnFork(ConnectionNumber(dpy));
                return;
            }
            debug("OpenDisplay(%s) attempt %d failed: %m\n", d->name, i + 1);
            sleep((unsigned)d->openDelay);
        } else {
            logError("Hung in XOpenDisplay(%s), aborting\n", d->name);
            (void)Signal(SIGALRM, SIG_DFL);
            break;
        }
    } while (++i < d->openRepeat);
    logError("Cannot connect to %s, giving up\n", d->name);
    exit(EX_OPENFAILED_DPY);
}


void
resetServer(struct display *d)
{
    if (dpy && (d->displayType & d_origin) != dFromXDMCP)
        pseudoReset();
}


static Jmp_buf pingTime;

static void
pingLost(void)
{
    Longjmp(pingTime, 1);
}

/* ARGSUSED */
static int
pingLostIOErr(Display *dspl ATTR_UNUSED)
{
    pingLost();
    return 0;
}

/* ARGSUSED */
static void
pingLostSig(int n ATTR_UNUSED)
{
    pingLost();
}

int
pingServer(struct display *d)
{
    int (*oldError)(Display *);
    void (*oldSig)(int);
    int oldAlarm;

    oldError = XSetIOErrorHandler(pingLostIOErr);
    oldAlarm = alarm(0);
    oldSig = Signal(SIGALRM, pingLostSig);
    (void)alarm(d->pingTimeout * 60);
    if (!Setjmp(pingTime)) {
        debug("ping X server\n");
        XSync(dpy, 0);
    } else {
        debug("X server dead\n");
        (void)alarm(0);
        (void)Signal(SIGALRM, SIG_DFL);
        XSetIOErrorHandler(oldError);
        return False;
    }
    (void)alarm(0);
    (void)Signal(SIGALRM, oldSig);
    (void)alarm(oldAlarm);
    debug("X server alive\n");
    XSetIOErrorHandler(oldError);
    return True;
}
