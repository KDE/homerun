/*

Copyright 1988, 1998  The Open Group
Copyright 2000-2004 Oswald Buddenhagen <ossi@kde.org>

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
 * maintain the authorization generation daemon
 */

#include "dm.h"
#include "dm_auth.h"
#include "dm_error.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include <sys/ioctl.h>

#include "dm_socket.h"
#ifdef DNETCONN
# include <netdnet/dnetdb.h>
#endif

/*
  This condition is copied verbatim from XlibInt.
  As since the demise of imake nobody is setting many of these variables
  any more, NEED_UTSNAME is essentially always defined. Who cares?
*/
#if (defined(_POSIX_SOURCE) && !defined(AIXV3) && !defined(__QNX__)) || defined(hpux) || defined(USG) || defined(SVR4)
# define NEED_UTSNAME
# include <sys/utsname.h>
#endif

#ifdef __GNU__
# include <netdb.h>
# undef SIOCGIFCONF
#else /* __GNU__ */
# include <net/if.h>
# ifdef __svr4__
#  include <netdb.h>
#  include <sys/sockio.h>
#  include <sys/stropts.h>
# endif
# ifdef __EMX__
#  define chown(a,b,c)
#  include <io.h>
# endif
#endif /* __GNU__ */

#include <X11/Xlib.h>

Xauth *
getAuthHelper(unsigned short namelen, const char *name)
{
    Xauth *new;

    if (!(new = Malloc(sizeof(*new))))
        return 0;

    if (!(new->name = Malloc(namelen))) {
        free(new);
        return 0;
    }
    new->name_length = namelen;
    memmove(new->name, name, namelen);

    new->family = FamilyWild;
    new->address_length = 0;
    new->address = 0;
    new->number_length = 0;
    new->number = 0;

    return new;
}

Xauth *
generateAuthHelper(unsigned short namelen, const char *name, int nbytes)
{
    Xauth *new;

    if (!(new = getAuthHelper(namelen, name)))
        return 0;

    if (!(new->data = Malloc(nbytes))) {
        free(new->name);
        free(new);
        return 0;
    }
    new->data_length = nbytes;
    if (!generateAuthData(new->data, nbytes)) {
        free(new->data);
        free(new->name);
        free(new);
        return 0;
    }

    return new;
}

#define MIT_AUTH_DATA_LEN 16 /* bytes of authorization data */

static Xauth *
mitGetAuth(unsigned short namelen, const char *name)
{
    return generateAuthHelper(namelen, name, MIT_AUTH_DATA_LEN);
}

struct AuthProtocol {
    unsigned short name_length;
    const char *name;
    Xauth *(*GetAuth)(unsigned short len, const char *name);
#ifdef XDMCP
    void (*GetXdmcpAuth)(struct protoDisplay *pdpy,
                         unsigned short authorizationNameLen,
                         const char *authorizationName);
#endif
};

#ifdef XDMCP
# define xdmcpauth(arg) , arg
#else
# define xdmcpauth(arg)
#endif

static const struct AuthProtocol authProtocols[] = {
{ (unsigned short)18, "MIT-MAGIC-COOKIE-1",
    mitGetAuth xdmcpauth(0)
},
#ifdef HASXDMAUTH
{ (unsigned short)19, "XDM-AUTHORIZATION-1",
    xdmGetAuth xdmcpauth(xdmGetXdmcpAuth)
},
#endif
#ifdef SECURE_RPC
{ (unsigned short)9, "SUN-DES-1",
    secureRPCGetAuth xdmcpauth(0)
},
#endif
#ifdef K5AUTH
{ (unsigned short)14, "MIT-KERBEROS-5",
    krb5GetAuth xdmcpauth(0)
},
#endif
};

static const struct AuthProtocol *
findProtocol(unsigned short name_length, const char *name)
{
    unsigned i;

    for (i = 0; i < as(authProtocols); i++)
        if (authProtocols[i].name_length == name_length &&
            memcmp(authProtocols[i].name, name, name_length) == 0)
        {
            return &authProtocols[i];
        }
    return 0;
}

int
validAuthorization(unsigned short name_length, const char *name)
{
    if (findProtocol(name_length, name))
        return True;
    return False;
}

static Xauth *
generateAuthorization(unsigned short name_length, const char *name)
{
    const struct AuthProtocol *a;
    Xauth *auth = 0;

    debug("generateAuthorization %s\n", name);
    if ((a = findProtocol(name_length, name))) {
        auth = (*a->GetAuth)(name_length, name);
        if (auth) {
            debug("got %p (%d %.*s) %02[*hhx\n", auth,
                  auth->name_length, auth->name_length, auth->name,
                  auth->data_length, auth->data);
        } else {
            debug("got (null)\n");
        }
    } else {
        debug("unknown authorization %s\n", name);
    }
    return auth;
}

#ifdef XDMCP

void
setProtoDisplayAuthorization(struct protoDisplay *pdpy,
                             unsigned short authorizationNameLen,
                             const char *authorizationName)
{
    const struct AuthProtocol *a;
    Xauth *auth;

    pdpy->xdmcpAuthorization = pdpy->fileAuthorization = 0;
    if ((a = findProtocol(authorizationNameLen, authorizationName))) {
        if (a->GetXdmcpAuth) {
            (*a->GetXdmcpAuth)(pdpy, authorizationNameLen, authorizationName);
            auth = pdpy->xdmcpAuthorization;
        } else {
            auth = (*a->GetAuth)(authorizationNameLen, authorizationName);
            pdpy->fileAuthorization = auth;
            pdpy->xdmcpAuthorization = 0;
        }
        if (auth)
            debug("got %p (%d %.*s)\n", auth,
                  auth->name_length, auth->name_length, auth->name);
        else
            debug("got (null)\n");
    }
}

#endif /* XDMCP */

SetAuthStatus
setDynamicDisplayAuthorization(struct display *d,
                               const char *authorizationName,
                               const char *authorizationData)
{
    Xauth *new;
    char *data;
    int dataLen;

    dataLen = strlen(authorizationData) / 2;
    if (!(data = Malloc(dataLen)))
        return SetAuthOOM;
    if (!hexToBinary(data, authorizationData)) {
        free(data);
        return SetAuthBad;
    }

    if (!(new = getAuthHelper(strlen(authorizationName),
                              authorizationName))) {
        free(data);
        return SetAuthOOM;
    }

    new->data = data;
    new->data_length = dataLen;

    if (!(d->authorizations = Malloc(sizeof(Xauth *)))) {
        free(new->name);
        free(new);
        free(data);
        return SetAuthOOM;
    }
    d->authorizations[0] = new;
    d->authNum = 1;
    return SetAuthOk;
}

void
cleanUpFileName(const char *src, char *dst, int len)
{
    while (*src) {
        if (--len <= 0)
            break;
        switch (*src & 0x7f) {
        case '/':
            *dst++ = '_';
            break;
        case '-':
            *dst++ = '.';
            break;
        default:
            *dst++ = (*src & 0x7f);
        }
        ++src;
    }
    *dst = '\0';
}


static FILE *
fdOpenW(int fd)
{
    FILE *f;

    if (fd >= 0) {
        if ((f = fdopen(fd, "w")))
            return f;
        close(fd);
    }
    return 0;
}


static FILE *
mkTempFile(char *nambuf, int namelen)
{
    FILE *f;
    int r;

    for (r = 0; r < 100; r++) {
        randomStr(nambuf + namelen);
        if ((f = fdOpenW(open(nambuf, O_WRONLY | O_CREAT | O_EXCL, 0600))))
            return f;
        if (errno != EEXIST)
            break;
    }
    return 0;
}

#define NAMELEN 255

static FILE *
makeServerAuthFile(struct display *d, char **authFile)
{
    FILE *f;
    int i;
    char cleanname[NAMELEN], nambuf[NAMELEN+128];

    /*
     * Some paranoid, but still not sufficient (DoS was still possible)
     * checks used to be here. I removed all this stuff because
     * a) authDir is supposed to be /var/run/xauth (=safe) or similar and
     * b) even if it's not (say, /tmp), we create files safely (hopefully).
     */
    if (mkdir(authDir, 0755) < 0  &&  errno != EEXIST)
        return 0;
    cleanUpFileName(d->name, cleanname, NAMELEN - 8);
    i = sprintf(nambuf, "%s/A%s-", authDir, cleanname);
    if ((f = mkTempFile(nambuf, i))) {
        strDup(authFile, nambuf);
        return f;
    }
    return 0;
}

static int
saveAuthorizations(struct display *d, char **authFile, Xauth **auths, int count)
{
    FILE *auth_file;
    int i;

    if (*authFile) {
        if (!(auth_file = fdOpenW(creat(*authFile, 0600)))) {
            logError("Cannot open X server authorization file %s\n", *authFile);
            free(*authFile);
            *authFile = 0;
            return False;
        }
    } else {
        if (!(auth_file = makeServerAuthFile(d, authFile))) {
            logError("Cannot create X server authorization file\n");
            return False;
        }
    }
    debug("file: %s  auth: %p\n", *authFile, auths);
    for (i = 0; i < count; i++) {
        /*
         * User-based auths may not have data until
         * a user logs in.  In which case don't write
         * to the auth file so xrdb and setup programs don't fail.
         */
        if (auths[i]->data_length > 0)
            if (!XauWriteAuth(auth_file, auths[i])) {
                fclose(auth_file);
                goto whoops;
            }
    }
    if (fclose(auth_file) == EOF) {
      whoops:
        logError("Cannot write X server authorization file %s: %m\n",
                 *authFile);
        unlink(*authFile);
        free(*authFile);
        *authFile = 0;
        return False;
    }
    return True;
}

int
saveGreeterAuthorizations(struct display *d)
{
    return saveAuthorizations(d, &d->greeterAuthFile,
                              d->authorizations, d->authNum);
}

int
saveServerAuthorizations(struct display *d, Xauth **auths, int count)
{
    if (!d->authFile && d->clientAuthFile && *d->clientAuthFile)
        strDup(&d->authFile, d->clientAuthFile);
    return saveAuthorizations(d, &d->authFile, auths, count);
}

void
setLocalAuthorization(struct display *d)
{
    Xauth *auth, **auths;
    int i, confAuths, gotAuths;

    if (d->authorizations) {
        for (i = 0; i < d->authNum; i++)
            XauDisposeAuth(d->authorizations[i]);
        free(d->authorizations);
        d->authorizations = 0;
        d->authNum = 0;
    }
    debug("setLocalAuthorization %s, auths %[s\n", d->name, d->authNames);
    confAuths = arrLen(d->authNames);
    if (!(auths = Malloc(confAuths * sizeof(Xauth *))))
        return;
    gotAuths = 0;
    for (i = 0; i < confAuths; i++) {
        auth = generateAuthorization(strlen(d->authNames[i]), d->authNames[i]);
        if (auth)
            auths[gotAuths++] = auth;
    }
    if (saveServerAuthorizations(d, auths, gotAuths)) {
        d->authorizations = auths;
        d->authNum = gotAuths;
    } else {
        for (i = 0; i < gotAuths; i++)
            XauDisposeAuth(auths[i]);
        free(auths);
    }
}

/*
 * Set the authorization to use for xdm's initial connection
 * to the X server.  Cannot use user-based authorizations
 * because no one has logged in yet, so we don't have any
 * user credentials.
 * Well, actually we could use SUN-DES-1 because we tell the server
 * to allow root in.  This is bogus and should be fixed.
 */
void
setAuthorization(struct display *d)
{
    register Xauth **auth = d->authorizations;
    int i;

    for (i = 0; i < d->authNum; i++) {
        if (auth[i]->name_length == 9 &&
                !memcmp(auth[i]->name, "SUN-DES-1", 9))
            continue;
        if (auth[i]->name_length == 14 &&
                !memcmp(auth[i]->name, "MIT-KERBEROS-5", 14))
            continue;
        XSetAuthorization(auth[i]->name, (int)auth[i]->name_length,
                          auth[i]->data, (int)auth[i]->data_length);
    }
}

static int
openFiles(const char *name, char *new_name, FILE **oldp, FILE **newp)
{
    strcat(strcpy(new_name, name), "-n");
    if (!(*newp = fdOpenW(creat(new_name, 0600)))) {
        debug("cannot open new file %s\n", new_name);
        return False;
    }
    *oldp = fopen(name, "r");
    debug("opens succeeded %s %s\n", name, new_name);
    return True;
}

struct addrList {
    struct addrList *next;
    unsigned short family, address_length, number_length;
    char data[1];
};

static struct addrList *addrs;

static void
initAddrs(void)
{
    addrs = 0;
}

static void
doneAddrs(void)
{
    struct addrList *a, *n;
    for (a = addrs; a; a = n) {
        n = a->next;
        free(a);
    }
}

static void
saveEntry(Xauth *auth)
{
    struct addrList *new;

    if (!(new = Malloc(offsetof(struct addrList, data) +
                       auth->address_length + auth->number_length)))
        return;
    new->address_length = auth->address_length;
    new->number_length = auth->number_length;
    memcpy(new->data, auth->address, (int)auth->address_length);
    memcpy(new->data + (int)auth->address_length,
           auth->number, (int)auth->number_length);
    new->family = auth->family;
    new->next = addrs;
    addrs = new;
}

static int
checkEntry(Xauth *auth)
{
    struct addrList *a;

    for (a = addrs; a; a = a->next)
        if (a->family == auth->family &&
            a->address_length == auth->address_length &&
            !memcmp(a->data, auth->address, auth->address_length) &&
            a->number_length == auth->number_length &&
            !memcmp(a->data + a->address_length,
                    auth->number, auth->number_length))
            return True;
    return False;
}

static void
writeAuth(FILE *file, Xauth *auth, int *ok)
{
    if (debugLevel & DEBUG_AUTH) /* normally too verbose */
        debug("writeAuth: doWrite = %d\n"
              "family: %d\n"
              "addr:    %02[*:hhx\n"
              "number:  %02[*:hhx\n"
              "name:    %02[*:hhx\n"
              "data:    %02[*:hhx\n",
              ok, auth->family,
              auth->address_length, auth->address,
              auth->number_length, auth->number,
              auth->name_length, auth->name,
              auth->data_length, auth->data);
    if (ok && !XauWriteAuth(file, auth))
        *ok = False;
}

static void
writeAddr(int family, int addr_length, CARD8 *addr,
          FILE *file, Xauth *auth, int *ok)
{
    auth->family = (unsigned short)family;
    auth->address_length = addr_length;
    auth->address = (char *)addr;
    debug("writeAddr: writing and saving an entry\n");
    writeAuth(file, auth, ok);
    if (!checkEntry(auth))
        saveEntry(auth);
}

static void
defineLocal(FILE *file, Xauth *auth, int *ok)
{
    char displayname[100];
#ifdef NEED_UTSNAME
    struct utsname name;
#endif

    /* stolen from xinit.c */

/* Make sure this produces the same string as _XGetHostname in lib/X/XlibInt.c.
 * Otherwise, Xau will not be able to find your cookies in the Xauthority file.
 *
 * Note: POSIX says that the ``nodename'' member of utsname does _not_ have
 *       to have sufficient information for interfacing to the network,
 *       and so, you may be better off using gethostname (if it exists).
 */

#ifdef NEED_UTSNAME
    /* hpux:
     * Why not use gethostname()?  Well, at least on my system, I've had to
     * make an ugly kernel patch to get a name longer than 8 characters, and
     * uname() lets me access to the whole string (it smashes release, you
     * see), whereas gethostname() kindly truncates it for me.
     */
    uname(&name);
    writeAddr(FamilyLocal, strlen(name.nodename), (CARD8 *)name.nodename,
              file, auth, ok);
#endif

    /*
     * For HP-UX, HP's Xlib expects a fully-qualified domain name, which
     * is achieved by using gethostname().  For compatibility, we must
     * also still create the entry using uname() above.
     */
    displayname[0] = 0;
    if (!gethostname(displayname, sizeof(displayname)))
        displayname[sizeof(displayname) - 1] = 0;

# ifdef NEED_UTSNAME
    /*
     * If gethostname and uname both returned the same name,
     * do not write a duplicate entry.
     */
    if (strcmp(displayname, name.nodename))
# endif
        writeAddr(FamilyLocal, strlen(displayname), (CARD8 *)displayname,
                  file, auth, ok);
}

#ifdef SYSV_SIOCGIFCONF

/* Deal with different SIOCGIFCONF ioctl semantics on SYSV, SVR4 */

int
ifioctl(int fd, int cmd, char *arg)
{
    struct strioctl ioc;
    int ret;

    bzero(&ioc, sizeof(ioc));
    ioc.ic_cmd = cmd;
    ioc.ic_timout = 0;
    if (cmd == SIOCGIFCONF) {
        ioc.ic_len = ((struct ifconf *)arg)->ifc_len;
        ioc.ic_dp = ((struct ifconf *)arg)->ifc_buf;
    } else {
        ioc.ic_len = sizeof(struct ifreq);
        ioc.ic_dp = arg;
    }
    ret = ioctl(fd, I_STR, (char *)&ioc);
    if (ret >= 0 && cmd == SIOCGIFCONF)
        ((struct ifconf *)arg)->ifc_len = ioc.ic_len;
    return (ret);
}

#endif /* SYSV_SIOCGIFCONF */

#ifdef HAVE_GETIFADDRS
# include <ifaddrs.h>

static void
defineSelf(FILE *file, Xauth *auth, int *ok)
{
    struct ifaddrs *ifap, *ifr;
    CARD8 *addr;
    int family, len;

    if (getifaddrs(&ifap) < 0)
        return;
    for (ifr = ifap; ifr; ifr = ifr->ifa_next) {
        if (!ifr->ifa_addr)
            continue;
        family = convertAddr((char *)(ifr->ifa_addr), &len, &addr);
        if (family == -1 || family == FamilyLocal)
            continue;
        /*
         * don't write out 'localhost' entries, as
         * they may conflict with other local entries.
         * defineLocal will always be called to add
         * the local entry anyway, so this one can
         * be tossed.
         */
        if (family == FamilyInternet &&
            addr[0] == 127 && addr[1] == 0 && addr[2] == 0 && addr[3] == 1)
        {
            debug("Skipping localhost address\n");
            continue;
        }
# if defined(IPv6) && defined(AF_INET6)
        if (family == FamilyInternet6) {
            if (IN6_IS_ADDR_LOOPBACK(((struct in6_addr *)addr))) {
                debug("Skipping IPv6 localhost address\n");
                continue;
            }
            /* Also skip XDM-AUTHORIZATION-1 */
            if (auth->name_length == 19 &&
                !memcmp(auth->name, "XDM-AUTHORIZATION-1", 19))
            {
                debug("Skipping IPv6 XDM-AUTHORIZATION-1\n");
                continue;
            }
        }
# endif
        writeAddr(family, len, addr, file, auth, ok);
    }
    freeifaddrs(ifap);
}
#else  /* GETIFADDRS */

#if defined(STREAMSCONN) && !defined(SYSV_SIOCGIFCONF) && !defined(WINTCP)

#include <tiuser.h>

/* Define this host for access control.  Find all the hosts the OS knows about
 * for this fd and add them to the selfhosts list.
 * TLI version, written without sufficient documentation.
 */
static void
defineSelf(int fd, FILE *file, Xauth *auth, int *ok)
{
    struct netbuf netb;
    char addrret[1024]; /* easier than t_alloc */

    netb.maxlen = sizeof(addrret);
    netb.buf = addrret;
    if (t_getname(fd, &netb, LOCALNAME) == -1)
        t_error("t_getname");
    /* what a kludge */
    writeAddr(FamilyInternet, 4, (CARD8 *)netb.buf + 4, file, auth, ok);
}

#else

#ifdef WINTCP /* NCR with Wollongong TCP */

#include <stropts.h>
#include <tiuser.h>

#include <sys/stream.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/in.h>
#include <netinet/in_var.h>

static void
defineSelf(int fd, FILE *file, Xauth *auth, int *ok)
{
    /*
     * The Wollongong drivers used by NCR SVR4/MP-RAS don't understand the
     * socket IO calls that most other drivers seem to like. Because of
     * this, this routine must be special cased for NCR. Eventually,
     * this will be cleared up.
     */

    struct ipb ifnet;
    struct in_ifaddr ifaddr;
    struct strioctl str;
    int len, ipfd;

    if ((ipfd = open("/dev/ip", O_RDWR, 0)) < 0) {
        logError("Trouble getting interface configuration\n");
        return;
    }

    /* Indicate that we want to start at the beginning */
    ifnet.ib_next = (struct ipb *)1;

    while (ifnet.ib_next) {
        str.ic_cmd = IPIOC_GETIPB;
        str.ic_timout = 0;
        str.ic_len = sizeof(struct ipb);
        str.ic_dp = (char *)&ifnet;

        if (ioctl(ipfd, (int)I_STR, (char *)&str) < 0) {
            close(ipfd);
            logError("Trouble getting interface configuration\n");
            return;
        }

        ifaddr.ia_next = (struct in_ifaddr *)ifnet.if_addrlist;
        str.ic_cmd = IPIOC_GETINADDR;
        str.ic_timout = 0;
        str.ic_len = sizeof(struct in_ifaddr);
        str.ic_dp = (char *)&ifaddr;

        if (ioctl(ipfd, (int)I_STR, (char *)&str) < 0) {
            close(ipfd);
            logError("Trouble getting interface configuration\n");
            return;
        }

        /*
         * Ignore the 127.0.0.1 entry.
         */
        if (IA_SIN(&ifaddr)->sin_addr.s_addr == htonl(0x7f000001))
            continue;

        writeAddr(FamilyInternet, 4, (CARD8 *)&(IA_SIN(&ifaddr)->sin_addr),
                  file, auth, ok);

    }
    close(ipfd);
}

#else /* WINTCP */

#if defined(SIOCGIFCONF) || defined (SIOCGLIFCONF)

#if !defined(SYSV_SIOCGIFCONF) || defined(SIOCGLIFCONF)
# define ifioctl ioctl
#endif

#ifdef SIOCGLIFCONF
# define ifr_type struct lifreq
#else
# define ifr_type struct ifreq
#endif

/* Handle variable length ifreq in BNR2 and later */
#ifdef VARIABLE_IFREQ
# define ifr_size(p) (sizeof(struct ifreq) + \
                      (p->ifr_addr.sa_len > sizeof(p->ifr_addr) ? \
                       p->ifr_addr.sa_len - sizeof(p->ifr_addr) : 0))
#else
# define ifr_size(p) (sizeof(ifr_type))
#endif

#ifdef SIOCGLIFCONF
# define IFC_IOCTL_REQ SIOCGLIFCONF
# define IFC_REQ(ifc) ifc.lifc_req
# define IFC_LEN(ifc) ifc.lifc_len
# define IFR_ADDR(ifr) ifr->lifr_addr
# define IFR_NAME(ifr) ifr->lifr_name
#else
# define IFC_IOCTL_REQ SIOCGIFCONF
# define IFC_REQ(ifc) ifc.ifc_req
# define IFC_LEN(ifc) ifc.ifc_len
# define IFR_ADDR(ifr) ifr->ifr_addr
# define IFR_NAME(ifr) ifr->ifr_name
#endif

/* Define this host for access control.  Find all the hosts the OS knows about
 * for this fd and add them to the selfhosts list.
 */
static void
defineSelf(int fd, FILE *file, Xauth *auth, int *ok)
{
    char buf[2048], *cp, *cplim;
    int len;
    CARD8 *addr;
    int family;
    ifr_type *ifr;
#ifdef SIOCGLIFCONF
    int n;
    void * bufptr = buf;
    size_t buflen = sizeof(buf);
    struct lifconf ifc;
# ifdef SIOCGLIFNUM
    struct lifnum ifn;
# endif
#else
    struct ifconf ifc;
#endif

#if defined(SIOCGLIFNUM) && defined(SIOCGLIFCONF)
    ifn.lifn_family = AF_UNSPEC;
    ifn.lifn_flags = 0;
    if (ioctl(fd, (int)SIOCGLIFNUM, (char *)&ifn) < 0)
        logError("Failed getting interface count\n");
    if (buflen < (ifn.lifn_count * sizeof(struct lifreq))) {
        buflen = ifn.lifn_count * sizeof(struct lifreq);
        bufptr = Malloc(buflen);
    }
#endif

#ifdef SIOCGLIFCONF
    ifc.lifc_family = AF_UNSPEC;
    ifc.lifc_flags = 0;
    ifc.lifc_len = buflen;
    ifc.lifc_buf = bufptr;
#else
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
#endif
    if (ifioctl(fd, IFC_IOCTL_REQ, (char *)&ifc) < 0) {
        logError("Trouble getting network interface configuration\n");
#if defined(SIOCGLIFNUM) && defined(SIOCGLIFCONF)
        if (bufptr != buf)
            free(bufptr);
#endif
        return;
    }

    cplim = (char *)IFC_REQ(ifc) + IFC_LEN(ifc);

    for (cp = (char *)IFC_REQ(ifc); cp < cplim; cp += ifr_size(ifr)) {
        ifr = (ifr_type *) cp;
#ifdef DNETCONN
        /*
         * this is ugly but SIOCGIFCONF returns decnet addresses in
         * a different form from other decnet calls
         */
        if (IFR_ADDR(ifr).sa_family == AF_DECnet) {
            len = sizeof(struct dn_naddr);
            addr = (CARD8 *)IFR_ADDR(ifr).sa_data;
            family = FamilyDECnet;
        } else
#endif
        {
            family = convertAddr((char *)&IFR_ADDR(ifr), &len, &addr);
            if (family < 0)
                continue;

            if (len == 0) {
                debug("skipping zero length address\n");
                continue;
            }
            /*
             * don't write out 'localhost' entries, as
             * they may conflict with other local entries.
             * defineLocal will always be called to add
             * the local entry anyway, so this one can
             * be tossed.
             */
            if (family == FamilyInternet &&
                addr[0] == 127 && addr[1] == 0 &&
                addr[2] == 0 && addr[3] == 1)
            {
                debug("skipping localhost address\n");
                continue;
            }
#if defined(IPv6) && defined(AF_INET6)
            if (family == FamilyInternet6) {
                if (IN6_IS_ADDR_LOOPBACK(((struct in6_addr *)addr))) {
                    debug("Skipping IPv6 localhost address\n");
                    continue;
                }
                /* Also skip XDM-AUTHORIZATION-1 */
                if (auth->name_length == 19 &&
                    !memcmp(auth->name, "XDM-AUTHORIZATION-1", 19))
                {
                    debug("Skipping IPv6 XDM-AUTHORIZATION-1\n");
                    continue;
                }
            }
#endif
        }
        debug("defineSelf: write network address, length %d\n", len);
        writeAddr(family, len, addr, file, auth, ok);
    }
#if defined(SIOCGLIFNUM) && defined(SIOCGLIFCONF)
    if (bufptr != buf)
        free(bufptr);
#endif
}

#else /* SIOCGIFCONF */

#include <sys/utsname.h>
#include <netdb.h>

/* Define this host for access control.  Find all the hosts the OS knows about
 * for this fd and add them to the selfhosts list.
 */
static void
defineSelf(int fd, int file, int auth, int *ok)
{
    int len;
    CARD8 *addr;
    int family;

    struct utsname name;
    register struct hostent  *hp;

    union {
        struct sockaddr   sa;
        struct sockaddr_in  in;
    } saddr;

    struct sockaddr_in *inetaddr;

    /* hpux:
     * Why not use gethostname()?  Well, at least on my system, I've had to
     * make an ugly kernel patch to get a name longer than 8 characters, and
     * uname() lets me access to the whole string (it smashes release, you
     * see), whereas gethostname() kindly truncates it for me.
     */
    uname(&name);
    if ((hp = gethostbyname(name.nodename))) {
        saddr.sa.sa_family = hp->h_addrtype;
        inetaddr = (struct sockaddr_in *)(&(saddr.sa));
        memmove(&inetaddr->sin_addr, hp->h_addr,
                (int)hp->h_length);
        if ((family = convertAddr(&(saddr.sa), &len, &addr)) >= 0)
            writeAddr(FamilyInternet, sizeof(inetaddr->sin_addr),
                      (CARD8 *)(&inetaddr->sin_addr), file, auth, ok);
    }
}

#endif /* SIOCGIFCONF else */
#endif /* WINTCP else */
#endif /* STREAMSCONN && !SYSV_SIOCGIFCONF else */
#endif /* HAVE_GETIFADDRS */

static void
setAuthNumber(Xauth *auth, const char *name)
{
    char *colon;
    char *dot;

    debug("setAuthNumber %s\n", name);
    colon = strrchr(name, ':');
    if (colon) {
        ++colon;
        dot = strchr(colon, '.');
        if (dot)
            auth->number_length = dot - colon;
        else
            auth->number_length = strlen(colon);
        if (!strNDup(&auth->number, colon, auth->number_length))
            auth->number_length = 0;
        debug("setAuthNumber: %s\n", auth->number);
    }
}

static void
writeLocalAuth(FILE *file, Xauth *auth, const char *name, int *ok)
{
#if defined(STREAMSCONN) || !defined(HAVE_GETIFADDRS)
    int fd;
#endif

    debug("writeLocalAuth: %s %.*s\n", name, auth->name_length, auth->name);
    setAuthNumber(auth, name);
# ifdef STREAMSCONN
    fd = t_open("/dev/tcp", O_RDWR, 0);
    t_bind(fd, 0, 0);
    defineSelf(fd, file, auth, ok);
    t_unbind(fd);
    t_close(fd);
# elif defined(HAVE_GETIFADDRS)
    defineSelf(file, auth, ok);
# else
#  ifdef TCPCONN
#   if defined(IPv6) && defined(AF_INET6)
    fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (fd < 0)
#   endif
    fd = socket(AF_INET, SOCK_STREAM, 0);
    defineSelf(fd, file, auth, ok);
    close(fd);
#  endif
#  ifdef DNETCONN
    fd = socket(AF_DECnet, SOCK_STREAM, 0);
    defineSelf(fd, file, auth, ok);
    close(fd);
#  endif
# endif /* HAVE_GETIFADDRS */
    defineLocal(file, auth, ok);
}

#ifdef XDMCP

/*
 * Call convertAddr(), and if it returns an IPv4 localhost, convert it
 * to a local display name.  Meets the _XTransconvertAddress's localhost
 * hack.
 */

static int
convertAuthAddr(char *saddr, int *len, CARD8 **addr)
{
    int ret = convertAddr(saddr, len, addr);
    if (ret == FamilyInternet &&
            ((struct in_addr *)*addr)->s_addr == htonl(0x7F000001L))
        ret = FamilyLocal;
    return ret;
}

static void
writeRemoteAuth(FILE *file, Xauth *auth, XdmcpNetaddr peer, int peerlen,
                const char *name, int *ok)
{
    int family = FamilyLocal;
    CARD8 *addr;

    debug("writeRemoteAuth: %s %.*s\n", name, auth->name_length, auth->name);
    if (!peer || peerlen < 2)
        return;
    setAuthNumber(auth, name);
    family = convertAuthAddr(peer, &peerlen, &addr);
    debug("writeRemoteAuth: family %d\n", family);
    if (family != FamilyLocal) {
        debug("writeRemoteAuth: %d, %02[*:hhx\n",
              family, peerlen, addr);
        writeAddr(family, peerlen, addr, file, auth, ok);
    } else
        writeLocalAuth(file, auth, name, ok);
}

#endif /* XDMCP */

#define NBSIZE 1024

static void
startUserAuth(char *buf, char *nbuf, FILE **old, FILE **new)
{
    const char *home;
    int lockStatus;

    initAddrs();
    *new = 0;
    if ((home = getEnv(userEnviron, "HOME")) && strlen(home) < NBSIZE - 12) {
        sprintf(buf, "%s/.Xauthority", home);
        debug("XauLockAuth %s\n", buf);
        lockStatus = XauLockAuth(buf, 1, 2, 10);
        debug("lock is %d\n", lockStatus);
        if (lockStatus == LOCK_SUCCESS)
            if (!openFiles(buf, nbuf, old, new))
                XauUnlockAuth(buf);
    }
    if (!*new)
        logWarn("Cannot update authorization file in home dir %s\n", home);
}

static int
endUserAuth(FILE *old, FILE *new, const char *nname, int ok)
{
    Xauth *entry;
    struct stat statb;

    if (old) {
        if (fstat(fileno(old), &statb) != -1)
            chmod(nname, (int)(statb.st_mode & 0777));
        /*SUPPRESS 560*/
        while ((entry = XauReadAuth(old))) {
            if (!checkEntry(entry)) {
                debug("writing an entry\n");
                writeAuth(new, entry, &ok);
            }
            XauDisposeAuth(entry);
        }
        fclose(old);
    }
    if (fclose(new) == EOF)
        ok = False;
    doneAddrs();
    return ok;
}

static void
undoUserAuth(const char *name, const char *new_name)
{
    logWarn("Cannot save user authorization in home dir\n");
    unlink(new_name);
    XauUnlockAuth(name);
}

static char *
moveUserAuth(const char *name, char *new_name, char *envname)
{
    if (rename(new_name, name)) {
        debug("rename %s => %s failed: %m\n", new_name, name);
        logError("Cannot move user authorization into place: %m\n");
        envname = new_name;
    } else {
        debug("new authorization moved into place\n");
    }
    XauUnlockAuth(name);
    return envname;
}

void
setUserAuthorization(struct display *d)
{
    FILE *old, *new;
    char *name;
    char *envname;
    Xauth **auths;
    int i, ok;
    int magicCookie;
    int data_len;
    char name_buf[NBSIZE], new_name[NBSIZE + 2];

    debug("setUserAuthorization\n");
    auths = d->authorizations;
    if (auths) {
        if (d->forceUserAuthDir)
            goto fallback;
        startUserAuth(name_buf, new_name, &old, &new);
        if (new) {
            envname = 0;
            name = name_buf;
        } else {
            /* 'old' is uninitialized here */
          fallback:

            if (strlen(d->userAuthDir) >= NBSIZE - 13)
                return;
            /*
             * Note, that we don't lock the auth file here, as it's
             * temporary - we can assume that we are the only ones
             * knowing about this file anyway.
             */
            i = sprintf(name_buf, "%s/.Xauth", d->userAuthDir);
            new = mkTempFile(name_buf, i);
            if (!new) {
                logError("Cannot create authorization file in %s: %m\n",
                         d->userAuthDir);
                return;
            }
            name = 0;
            envname = name_buf;
            old = 0;
        }
        ok = True;
        debug("%d authorization protocols for %s\n", d->authNum, d->name);
        /*
         * Write MIT-MAGIC-COOKIE-1 authorization first, so that
         * R4 clients which only knew that, and used the first
         * matching entry will continue to function
         */
        magicCookie = -1;
        for (i = 0; i < d->authNum; i++) {
            if (auths[i]->name_length == 18 &&
                !memcmp(auths[i]->name, "MIT-MAGIC-COOKIE-1", 18))
            {
                magicCookie = i;
                if ((d->displayType & d_location) == dLocal)
                    writeLocalAuth(new, auths[i], d->name, &ok);
#ifdef XDMCP
                else
                    writeRemoteAuth(new, auths[i], (XdmcpNetaddr)d->peer.data,
                                    d->peer.length, d->name, &ok);
#endif
                break;
            }
        }
        /* now write other authorizations */
        for (i = 0; i < d->authNum; i++) {
            if (i != magicCookie) {
                data_len = auths[i]->data_length;
                /* client will just use default Kerberos cache, so don't
                 * even write cache info into the authority file.
                 */
                if (auths[i]->name_length == 14 &&
                        !strncmp(auths[i]->name, "MIT-KERBEROS-5", 14))
                    auths[i]->data_length = 0;
                if ((d->displayType & d_location) == dLocal)
                    writeLocalAuth(new, auths[i], d->name, &ok);
#ifdef XDMCP
                else
                    writeRemoteAuth(new, auths[i], (XdmcpNetaddr)d->peer.data,
                                    d->peer.length, d->name, &ok);
#endif
                auths[i]->data_length = data_len;
            }
        }
        if (!endUserAuth(old, new, new_name, ok)) {
            if (!name) {
                /* XXX this should be user-visible */
                logError("Cannot save user authorization: %m\n");
                return;
            }
            undoUserAuth(name, new_name);
            initAddrs();
            goto fallback;
        }
        if (name)
            envname = moveUserAuth(name, new_name, envname);
        if (envname) {
            userEnviron = setEnv(userEnviron, "XAUTHORITY", envname);
            systemEnviron = setEnv(systemEnviron, "XAUTHORITY", envname);
        }
        /* a chown() used to be here, but this code runs as user anyway */
    }
    debug("done setUserAuthorization\n");
}

void
removeUserAuthorization(struct display *d)
{
    Xauth **auths;
    FILE *old, *new;
    int i;
    char name[NBSIZE], new_name[NBSIZE + 2];

    if (!(auths = d->authorizations))
        return;
    debug("removeUserAuthorization\n");
    startUserAuth(name, new_name, &old, &new);
    if (new) {
        for (i = 0; i < d->authNum; i++) {
            if ((d->displayType & d_location) == dLocal)
                writeLocalAuth(new, auths[i], d->name, 0);
#ifdef XDMCP
            else
                writeRemoteAuth(new, auths[i], (XdmcpNetaddr)d->peer.data,
                                d->peer.length, d->name, 0);
#endif
        }
        if (endUserAuth(old, new, new_name, True))
            (void)moveUserAuth(name, new_name, 0);
        else
            undoUserAuth(name, new_name);
    }
    debug("done removeUserAuthorization\n");
}
