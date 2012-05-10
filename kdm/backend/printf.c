/*

Copyright 2001,2002,2004 Oswald Buddenhagen <ossi@kde.org>

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
 * printf.c - working horse of error.c
 */

/*
 * NOTE: this file is meant to be included, not linked,
 * so it can be used in the helper programs without much voodoo.
 */

/* ########## printf core implementation with some extensions ########## */
/*
 * How to use the extensions:
 * - put ' or " in the flags field to quote a string with this char and
 *   escape special characters (only available, if PRINT_QUOTES is defined)
 * - put \\ in the flags field to quote special characters and leading and
 *   trailing spaces (only available, if PRINT_QUOTES is defined)
 * - arrays (only available, if PRINT_ARRAYS is defined)
 *   - the array modifier [ comes after the maximal field width specifier
 *   - the array length can be specified literally, with the '*' modifier
 *     (in which case an argument is expected) or will be automatically
 *     determined (stop values are -1 for ints and 0 for strings)
 *   - these modifiers expect their argument to be an in-line string quoted
 *     with an arbitrary character:
 *     - (,) -> array pre-/suf-fix; default ""
 *     - <, > -> element pre-/suf-fix; default ""
 *     - | -> element separator; default " "
 *   - these modifiers expect no argument:
 *     - : -> print '<number of elements>: ' before an array
 *     - , -> short for |','
 *     - { -> short for ('{')' }'<' '|''
 *   - the pointer to the array is the last argument to the format
 * - the %m conversion from syslog() is supported
 *   (extended by -ENOSPC meaning "partial write")
 */

/**************************************************************
 * Partially stolen from OpenSSH's OpenBSD compat directory.
 * (C) Patrick Powell, Brandon Long, Thomas Roessler,
 *     Michael Elkins, Ben Lindstrom
 **************************************************************/

#include <ctype.h>
#include <string.h>
#include <stdarg.h>

/* format flags - Bits */
#define DP_F_MINUS     (1 << 0)
#define DP_F_PLUS      (1 << 1)
#define DP_F_SPACE     (1 << 2)
#define DP_F_NUM       (1 << 3)
#define DP_F_ZERO      (1 << 4)
#define DP_F_UPCASE    (1 << 5)
#define DP_F_UNSIGNED  (1 << 6)
#define DP_F_SQUOTE    (1 << 7)
#define DP_F_DQUOTE    (1 << 8)
#define DP_F_BACKSL    (1 << 9)
#define DP_F_ARRAY     (1 << 10)
#define DP_F_COLON     (1 << 11)

/* Conversion Flags */
#define DP_C_INT    0
#define DP_C_BYTE   1
#define DP_C_SHORT  2
#define DP_C_LONG   3
#define DP_C_STR    10

typedef void (*OutCh)(void *bp, char c);


static void
fmtint(OutCh dopr_outch, void *bp,
       long value, int base, int min, int max, int flags)
{
    const char *ctab;
    unsigned long uvalue;
    int signvalue = 0;
    int place = 0;
    int spadlen = 0; /* amount to space pad */
    int zpadlen = 0; /* amount to zero pad */
    char convert[20];

    if (max < 0)
        max = 0;

    uvalue = value;

    if (!(flags & DP_F_UNSIGNED)) {
        if (value < 0) {
            signvalue = '-';
            uvalue = -value;
        } else if (flags & DP_F_PLUS) { /* Do a sign (+/i) */
            signvalue = '+';
        } else if (flags & DP_F_SPACE) {
            signvalue = ' ';
        }
    }

    ctab = (flags & DP_F_UPCASE) ? "0123456789ABCDEF" : "0123456789abcdef";
    do {
        convert[place++] = ctab[uvalue % (unsigned)base];
        uvalue = uvalue / (unsigned)base;
    } while (uvalue);

    zpadlen = max - place;
    spadlen = min - (max > place ? max : place) -
              (signvalue ? 1 : 0) - ((flags & DP_F_NUM) ? 2 : 0);
    if (zpadlen < 0)
        zpadlen = 0;
    if (spadlen < 0)
        spadlen = 0;
    if (flags & DP_F_ZERO) {
        zpadlen = zpadlen > spadlen ? zpadlen : spadlen;
        spadlen = 0;
    }
    if (flags & DP_F_MINUS)
        spadlen = -spadlen; /* Left Justifty */


    /* Spaces */
    while (spadlen > 0) {
        dopr_outch(bp, ' ');
        --spadlen;
    }

    /* Sign */
    if (signvalue)
        dopr_outch(bp, signvalue);

    /* Prefix */
    if (flags & DP_F_NUM) {
        dopr_outch(bp, '0');
        dopr_outch(bp, 'x');
    }

    /* Zeros */
    if (zpadlen > 0)
        while (zpadlen > 0) {
            dopr_outch(bp, '0');
            --zpadlen;
        }

    /* Digits */
    while (place > 0)
        dopr_outch(bp, convert[--place]);

    /* Left Justified spaces */
    while (spadlen < 0) {
        dopr_outch(bp, ' ');
        ++spadlen;
    }
}

typedef struct {
    const char *str;
    size_t len;
} str_t;

static void
putstr(OutCh dopr_outch, void *bp, str_t *st)
{
    size_t pt;

    for (pt = 0; pt < st->len; pt++)
        dopr_outch(bp, st->str[pt]);
}

static str_t _null_parents = { "(null)", 6 };
#ifdef PRINT_ARRAYS
static str_t _null_dparents = { "((null))", 8 };
#endif
#if defined(PRINT_QUOTES) || defined(PRINT_ARRAYS)
static str_t _null_caps = { "NULL", 4 };
#endif

static void
fmtstr(OutCh dopr_outch, void *bp,
       const char *value, int flags, int min, int max)
{
    int padlen, strln, curcol;
#ifdef PRINT_QUOTES
    int lastcol;
#endif
    char ch;

    if (!value) {
#ifdef PRINT_QUOTES
        if (flags & (DP_F_SQUOTE | DP_F_DQUOTE))
            putstr(dopr_outch, bp, &_null_caps);
        else
#endif
            putstr(dopr_outch, bp, &_null_parents);
        return;
    }

    for (strln = 0; (unsigned)strln < (unsigned)max && value[strln]; strln++);
    padlen = min - strln;
    if (padlen < 0)
        padlen = 0;
    if (flags & DP_F_MINUS)
        padlen = -padlen; /* Left Justify */

    for (; padlen > 0; padlen--)
        dopr_outch(bp, ' ');
#ifdef PRINT_QUOTES
    lastcol = 0;
    if (flags & DP_F_SQUOTE)
        dopr_outch(bp, '\'');
    else if (flags & DP_F_DQUOTE)
        dopr_outch(bp, '"');
    else if (flags & DP_F_BACKSL)
        for (lastcol = strln; lastcol && value[lastcol - 1] == ' '; lastcol--);
#endif
    for (curcol = 0; curcol < strln; curcol++) {
        ch = value[curcol];
#ifdef PRINT_QUOTES
        if (flags & (DP_F_SQUOTE | DP_F_DQUOTE | DP_F_BACKSL)) {
            switch (ch) {
            case '\r': ch = 'r'; break;
            case '\n': ch = 'n'; break;
            case '\t': ch = 't'; break;
            case '\a': ch = 'a'; break;
            case '\b': ch = 'b'; break;
            case '\v': ch = 'v'; break;
            case '\f': ch = 'f'; break;
            default:
                if (ch < 32 ||
                    ((unsigned char)ch >= 0x7f && (unsigned char)ch < 0xa0))
                {
                    dopr_outch(bp, '\\');
                    fmtint(dopr_outch, bp, (unsigned char)ch, 8, 3, 3, DP_F_ZERO);
                    continue;
                } else {
                    if ((ch == '\'' && (flags & DP_F_SQUOTE)) ||
                        (ch == '"' && (flags & DP_F_DQUOTE)) ||
                        (ch == ' ' && (flags & DP_F_BACKSL) &&
                         (!curcol || curcol >= lastcol)) ||
                        ch == '\\')
                    {
                        dopr_outch(bp, '\\');
                    }
                    dopr_outch(bp, ch);
                    continue;
                }
            }
            dopr_outch(bp, '\\');
        }
#endif
        dopr_outch(bp, ch);
    }
#ifdef PRINT_QUOTES
    if (flags & DP_F_SQUOTE)
        dopr_outch(bp, '\'');
    else if (flags & DP_F_DQUOTE)
        dopr_outch(bp, '"');
#endif
    for (; padlen < 0; padlen++)
        dopr_outch(bp, ' ');
}

static void
doPrint(OutCh dopr_outch, void *bp, const char *format, va_list args)
{
    const char *strvalue;
#ifdef PRINT_ARRAYS
    str_t arpr, arsf, arepr, aresf, aresp, *arp;
    void *arptr;
#endif
    unsigned long value;
    int radix, min, max, flags, cflags, errn;
#ifdef PRINT_ARRAYS
    int arlen;
    unsigned aridx;
    char sch;
#endif
    char ch;
#define NCHR if (!(ch = *format++)) return

# ifdef PRINT_ARRAYS
    arlen = 0;
# endif
    radix = 0;
    errn = errno;
    for (;;) {
        for (;;) {
            NCHR;
            if (ch == '%')
                break;
            dopr_outch(bp, ch);
        }
        flags = cflags = min = 0;
        max = -1;
        for (;;) {
            NCHR;
            switch (ch) {
            case '#': flags |= DP_F_NUM; continue;
            case '-': flags |= DP_F_MINUS; continue;
            case '+': flags |= DP_F_PLUS; continue;
            case ' ': flags |= DP_F_SPACE; continue;
            case '0': flags |= DP_F_ZERO; continue;
#ifdef PRINT_QUOTES
            case '"': flags |= DP_F_DQUOTE; continue;
            case '\'': flags |= DP_F_SQUOTE; continue;
            case '\\': flags |= DP_F_BACKSL; continue;
#endif
            }
            break;
        }
        for (;;) {
            if (isdigit((unsigned char)ch)) {
                min = 10 * min + (ch - '0');
                NCHR;
                continue;
            } else if (ch == '*') {
                min = va_arg(args, int);
                NCHR;
            }
            break;
        }
        if (ch == '.') {
            max = 0;
            for (;;) {
                NCHR;
                if (isdigit((unsigned char)ch)) {
                    max = 10 * max + (ch - '0');
                    continue;
                } else if (ch == '*') {
                    max = va_arg(args, int);
                    NCHR;
                }
                break;
            }
        }
#ifdef PRINT_ARRAYS
        if (ch == '[') {
            flags |= DP_F_ARRAY;
            arlen = -1;
            arpr.len = arsf.len = arepr.len = aresf.len = 0;
            aresp.len = 1, aresp.str = " ";
            for (;;) {
                NCHR;
                if (isdigit((unsigned char)ch)) {
                    arlen = 0;
                    for (;;) {
                        arlen += (ch - '0');
                        NCHR;
                        if (!isdigit((unsigned char)ch))
                            break;
                        arlen *= 10;
                    }
                }
                switch (ch) {
                case ':': flags |= DP_F_COLON; continue;
                case '*': arlen = va_arg(args, int); continue;
                case '(': arp = &arpr; goto rar;
                case ')': arp = &arsf; goto rar;
                case '<': arp = &arepr; goto rar;
                case '>': arp = &aresf; goto rar;
                case '|': arp = &aresp;
                  rar:
                    NCHR;
                    sch = ch;
                    arp->str = format;
                    do {
                        NCHR;
                    } while (ch != sch);
                    arp->len = format - arp->str - 1;
                    continue;
                case ',':
                    aresp.len = 1, aresp.str = ",";
                    continue;
                case '{':
                    aresp.len = 0, arpr.len = arepr.len = 1, arsf.len = 2;
                    arpr.str = "{", arepr.str = " ", arsf.str = " }";
                    continue;
                }
                break;
            }
        }
#endif
        for (;;) {
            switch (ch) {
            case 'h':
                cflags = DP_C_SHORT;
                NCHR;
                if (ch == 'h') {
                    cflags = DP_C_BYTE;
                    NCHR;
                }
                continue;
            case 'l':
                cflags = DP_C_LONG;
                NCHR;
                continue;
            }
            break;
        }
        switch (ch) {
        case '%':
            dopr_outch(bp, ch);
            break;
        case 'm':
            strvalue = (errn == -ENOSPC) ? "partial write" : strerror(errn);
            fmtstr(dopr_outch, bp, strvalue, flags, min, max);
            break;
        case 'c':
            dopr_outch(bp, va_arg(args, int));
            break;
        case 's':
#ifdef PRINT_ARRAYS
            cflags = DP_C_STR;
            goto printit;
#else
            strvalue = va_arg(args, char *);
            fmtstr(dopr_outch, bp, strvalue, flags, min, max);
            break;
#endif
        case 'u':
            flags |= DP_F_UNSIGNED;
        case 'd':
        case 'i':
            radix = 10;
            goto printit;
        case 'X':
            flags |= DP_F_UPCASE;
        case 'x':
            flags |= DP_F_UNSIGNED;
            radix = 16;
          printit:
#ifdef PRINT_ARRAYS
            if (flags & DP_F_ARRAY) {
                if (!(arptr = va_arg(args, void *))) {
                    putstr(dopr_outch, bp,
                           arpr.len ? &_null_caps : &_null_dparents);
                } else {
                    if (arlen == -1) {
                        arlen = 0;
                        switch (cflags) {
                        case DP_C_STR: while (((char **)arptr)[arlen]) arlen++; break;
                        case DP_C_BYTE: while (((unsigned char *)arptr)[arlen] != (unsigned char)-1) arlen++; break;
                        case DP_C_SHORT: while (((unsigned short int *)arptr)[arlen] != (unsigned short int)-1) arlen++; break;
                        case DP_C_LONG: while (((unsigned long int *)arptr)[arlen] != (unsigned long int)-1) arlen++; break;
                        default: while (((unsigned int *)arptr)[arlen] != (unsigned int)-1) arlen++; break;
                        }
                    }
                    if (flags & DP_F_COLON) {
                        fmtint(dopr_outch, bp, (long)arlen, 10, 0, -1, DP_F_UNSIGNED);
                        dopr_outch(bp, ':');
                        dopr_outch(bp, ' ');
                    }
                    putstr(dopr_outch, bp, &arpr);
                    for (aridx = 0; aridx < (unsigned)arlen; aridx++) {
                        if (aridx)
                            putstr(dopr_outch, bp, &aresp);
                        putstr(dopr_outch, bp, &arepr);
                        if (cflags == DP_C_STR) {
                            strvalue = ((char **)arptr)[aridx];
                            fmtstr(dopr_outch, bp, strvalue, flags, min, max);
                        } else {
                            if (flags & DP_F_UNSIGNED) {
                                switch (cflags) {
                                case DP_C_BYTE: value = ((unsigned char *)arptr)[aridx]; break;
                                case DP_C_SHORT: value = ((unsigned short int *)arptr)[aridx]; break;
                                case DP_C_LONG: value = ((unsigned long int *)arptr)[aridx]; break;
                                default: value = ((unsigned int *)arptr)[aridx]; break;
                                }
                            } else {
                                switch (cflags) {
                                case DP_C_BYTE: value = ((signed char *)arptr)[aridx]; break;
                                case DP_C_SHORT: value = ((short int *)arptr)[aridx]; break;
                                case DP_C_LONG: value = ((long int *)arptr)[aridx]; break;
                                default: value = ((int *)arptr)[aridx]; break;
                                }
                            }
                            fmtint(dopr_outch, bp, value, radix, min, max, flags);
                        }
                        putstr(dopr_outch, bp, &aresf);
                    }
                    putstr(dopr_outch, bp, &arsf);
                }
            } else {
                if (cflags == DP_C_STR) {
                    strvalue = va_arg(args, char *);
                    fmtstr(dopr_outch, bp, strvalue, flags, min, max);
                } else {
#endif
                    if (flags & DP_F_UNSIGNED) {
                        switch (cflags) {
                        case DP_C_LONG: value = va_arg(args, unsigned long int); break;
                        default: value = va_arg(args, unsigned int); break;
                        }
                    } else {
                        switch (cflags) {
                        case DP_C_LONG: value = va_arg(args, long int); break;
                        default: value = va_arg(args, int); break;
                        }
                    }
                    fmtint(dopr_outch, bp, value, radix, min, max, flags);
#ifdef PRINT_ARRAYS
                }
            }
#endif
            break;
        case 'p':
            value = (long)va_arg(args, void *);
            fmtint(dopr_outch, bp, value, 16, sizeof(long) * 2 + 2,
                   max, flags | DP_F_UNSIGNED | DP_F_ZERO | DP_F_NUM);
            break;
        }
    }
}

/* ########## end of printf core implementation ########## */


/*
 * Logging function for xdm and helper programs.
 */
#ifndef NO_LOGGER

#include <stdio.h>
#include <time.h>

#ifdef USE_SYSLOG
# include <syslog.h>
# ifdef LOG_NAME
#  define InitLog() openlog(LOG_NAME, LOG_PID, LOG_DAEMON)
# else
#  define InitLog() openlog(prog, LOG_PID, LOG_DAEMON)
# endif
static int lognums[] = { LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERR, LOG_CRIT };
#else
# define InitLog() while(0)
#endif

static const char *lognams[] = { "debug", "info", "warning", "error", "panic" };

static void
logTime(char *dbuf)
{
    time_t tim;
    (void)time(&tim);
    strftime(dbuf, 20, "%b %e %H:%M:%S", localtime(&tim));
}

#if defined(LOG_DEBUG_MASK) || defined(USE_SYSLOG)
STATIC int debugLevel;
#endif

#define OOMSTR "Out of memory. Expect problems.\n"

STATIC void
logOutOfMem(void)
{
    static time_t last;
    time_t tnow;

    time(&tnow);
    if (last + 100 > tnow) { /* don't log bursts */
        last = tnow;
        return;
    }
    last = tnow;
#ifdef USE_SYSLOG
    if (!(debugLevel & DEBUG_NOSYSLOG))
        syslog(LOG_CRIT, OOMSTR);
    else
#endif
    {
        int el;
        char dbuf[24], sbuf[128];
        logTime(dbuf);
        el = sprintf(sbuf, "%s "
#ifdef LOG_NAME
                     LOG_NAME "[%ld]: " OOMSTR, dbuf,
#else
                     "%s[%ld]: " OOMSTR, dbuf, prog,
#endif
                     (long)getpid());
        write(2, sbuf, el);
    }
}

typedef struct {
    char *buf;
    int clen, blen, type;
    char lmbuf[128];
} OCLBuf;

static void
flush_OCL(OCLBuf *oclbp)
{
    if (oclbp->clen) {
#ifdef USE_SYSLOG
        if (!(debugLevel & DEBUG_NOSYSLOG))
            syslog(lognums[oclbp->type], "%.*s", oclbp->clen, oclbp->buf);
        else
#endif
        {
            oclbp->buf[oclbp->clen] = '\n';
            write(2, oclbp->buf, oclbp->clen + 1);
        }
        oclbp->clen = 0;
    }
}

static void
outCh_OCL(void *bp, char c)
{
    OCLBuf *oclbp = (OCLBuf *)bp;
    char *nbuf;
    int nlen;

    if (c == '\n') {
        flush_OCL(oclbp);
    } else {
        if (oclbp->clen >= oclbp->blen - 1) {
            if (oclbp->buf == oclbp->lmbuf) {
                flush_OCL(oclbp);
                oclbp->buf = 0;
                oclbp->blen = 0;
            }
            nlen = oclbp->blen * 3 / 2 + 128;
            nbuf = Realloc(oclbp->buf, nlen);
            if (nbuf) {
                oclbp->buf = nbuf;
                oclbp->blen = nlen;
            } else {
                flush_OCL(oclbp);
                oclbp->buf = oclbp->lmbuf;
                oclbp->blen = sizeof(oclbp->lmbuf);
            }
        }
#ifdef USE_SYSLOG
        if (!oclbp->clen && (debugLevel & DEBUG_NOSYSLOG)) {
#else
        if (!oclbp->clen) {
#endif
            char dbuf[24];
            logTime(dbuf);
            oclbp->clen = sprintf(oclbp->buf, "%s "
#ifdef LOG_NAME
                                  LOG_NAME "[%ld] %s: ", dbuf,
#else
                                  "%s[%ld] %s: ", dbuf, prog,
#endif
                                  (long)getpid(), lognams[oclbp->type]);
        }
        oclbp->buf[oclbp->clen++] = c;
    }
}

static void
logger(int type, const char *fmt, va_list args)
{
    OCLBuf oclb;

    oclb.buf = 0;
    oclb.blen = oclb.clen = 0;
    oclb.type = type;
    doPrint(outCh_OCL, &oclb, fmt, args);
    /* no flush, every message is supposed to be \n-terminated */
    if (oclb.buf && oclb.buf != oclb.lmbuf)
        free(oclb.buf);
}

#ifdef LOG_DEBUG_MASK
STATIC void
debug(const char *fmt, ...)
{
    if (debugLevel & LOG_DEBUG_MASK) {
        va_list args;
        int olderrno = errno;
        va_start(args, fmt);
        logger(DM_DEBUG, fmt, args);
        va_end(args);
        errno = olderrno;
    }
}
#endif

#ifndef LOG_NO_INFO
STATIC void
logInfo(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    logger(DM_INFO, fmt, args);
    va_end(args);
}
#endif

#ifndef LOG_NO_WARN
STATIC void
logWarn(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    logger(DM_WARN, fmt, args);
    va_end(args);
}
#endif

#ifndef LOG_NO_ERROR
STATIC void
logError(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    logger(DM_ERR, fmt, args);
    va_end(args);
}
#endif

#ifdef LOG_PANIC_EXIT
STATIC void
logPanic(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    logger(DM_PANIC, fmt, args);
    va_end(args);
    exit(LOG_PANIC_EXIT);
}
#endif

#endif /* NO_LOGGER */

#ifdef NEED_FDPRINTF

typedef struct {
    char *buf;
    int clen, blen, tlen;
} OCFBuf;

static void
outCh_OCF(void *bp, char c)
{
    OCFBuf *ocfbp = (OCFBuf *)bp;
    char *nbuf;
    int nlen;

    ocfbp->tlen++;
    if (ocfbp->clen >= ocfbp->blen) {
        if (ocfbp->blen < 0)
            return;
        nlen = ocfbp->blen * 3 / 2 + 100;
        nbuf = Realloc(ocfbp->buf, nlen);
        if (!nbuf) {
            free(ocfbp->buf);
            ocfbp->blen = -1;
            ocfbp->buf = 0;
            ocfbp->clen = 0;
            return;
        }
        ocfbp->blen = nlen;
        ocfbp->buf = nbuf;
    }
    ocfbp->buf[ocfbp->clen++] = c;
}

STATIC int
FdPrintf(int fd, const char *fmt, ...)
{
    va_list args;
    OCFBuf ocfb = { 0, 0, 0, -1 };

    va_start(args, fmt);
    doPrint(outCh_OCF, &ocfb, fmt, args);
    va_end(args);
    if (ocfb.buf) {
        debug("FdPrintf %\".*s to %d\n", ocfb.clen, ocfb.buf, fd);
        (void)write(fd, ocfb.buf, ocfb.clen);
        free(ocfb.buf);
    }
    return ocfb.tlen;
}

#endif /* NEED_FDPRINTF */

#ifdef NEED_ASPRINTF

typedef struct {
    char *buf;
    int clen, blen, tlen;
} OCABuf;

static void
outCh_OCA(void *bp, char c)
{
    OCABuf *ocabp = (OCABuf *)bp;
    char *nbuf;
    int nlen;

    ocabp->tlen++;
    if (ocabp->clen >= ocabp->blen) {
        if (ocabp->blen < 0)
            return;
        nlen = ocabp->blen * 3 / 2 + 100;
        nbuf = Realloc(ocabp->buf, nlen);
        if (!nbuf) {
            free(ocabp->buf);
            ocabp->blen = -1;
            ocabp->buf = 0;
            ocabp->clen = 0;
            return;
        }
        ocabp->blen = nlen;
        ocabp->buf = nbuf;
    }
    ocabp->buf[ocabp->clen++] = c;
}

STATIC int
VASPrintf(char **strp, const char *fmt, va_list args)
{
    OCABuf ocab = { 0, 0, 0, -1 };

    doPrint(outCh_OCA, &ocab, fmt, args);
    outCh_OCA(&ocab, 0);
    *strp = Realloc(ocab.buf, ocab.clen);
    if (!*strp)
        *strp = ocab.buf;
    return ocab.tlen;
}

STATIC int
ASPrintf(char **strp, const char *fmt, ...)
{
    va_list args;
    int len;

    va_start(args, fmt);
    len = VASPrintf(strp, fmt, args);
    va_end(args);
    return len;
}

#endif /* NEED_ASPRINTF */
