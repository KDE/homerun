/*

Copyright 2003 Oswald Buddenhagen <ossi@kde.org>

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
 * load, save and manipulate ini-style config files
 */

#include "dm.h"
#include "dm_error.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

char *
iniLoad(const char *fname)
{
    char *data;
    int fd, len;
    struct stat st;

    if ((fd = open(fname, O_RDONLY | O_NONBLOCK)) < 0) {
        debug("cannot open ini-file %\"s: %m", fname);
        return 0;
    }
    if (fstat(fd, &st) || !S_ISREG(st.st_mode)) {
        logWarn("Ini-file %\"s is no regular file\n", fname);
        close(fd);
        return 0;
    }
    if (st.st_size >= 0x10000) {
        logWarn("Ini-file %\"s is too big\n", fname);
        close(fd);
        return 0;
    }
    len = st.st_size;
    if (!(data = Malloc(len + 2))) {
        close(fd);
        return 0;
    }
    if (read(fd, data, len) != len) {
        debug("cannot read ini-file %\"s: %m", fname);
        free(data);
        close(fd);
        return 0;
    }
    close(fd);
    if (data[len - 1] != '\n') /* iniEntry() needs this. */
        data[len++] = '\n';
    data[len] = 0;
    return data;
}

int
iniSave(const char *data, const char *fname)
{
    int fd, cnt, len;

    if ((fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK, 0600)) < 0) {
        debug("cannot create ini-file %\"s: %m", fname);
        return False;
    }
    len = strlen(data);
    if ((cnt = write(fd, data, len)) != len) {
        if (cnt != -1)
            errno = -ENOSPC;
        debug("cannot write ini-file %\"s: %m", fname);
        close(fd);
        return False;
    }
    if (close(fd) < 0) {
        debug("cannot write ini-file %\"s: %m", fname);
        return False;
    }
    return True;
}

#define apparr(d,s,n) do { memcpy (d, s, n); d += n; } while(0)
#define appbyte(d,b) *d++ = b

/*
 * NOTE: this relies on the last line being \n-terminated - if it is not,
 * it will read past the end of the buffer!
 */
char *
iniEntry(char *data, const char *section, const char *key, const char *value)
{
    char *p = data, *secinsert = 0, *pastinsert = 0, *cb, *ce, *ndata;
    const char *t;
    int insect = False, ll, sl, kl, vl, len, nlen;

    if (p) {
        while (*p) {
            for (; *p == ' ' || *p == '\t'; p++);
            if (*p == '\n') {
                /* Empty line, so don't advance insertion point. */
                p++;
                continue;
            }
            if (*p == '[') {
                for (t = section; *++p == *t; t++);
                insect = !*t && *p == ']';
            } else if (insect) {
                for (t = key; *p == *t; t++, p++);
                for (; *p == ' ' || *p == '\t'; p++);
                if (!*t && *p == '=') {
                    for (p++; *p == ' ' || *p == '\t'; p++);
                    cb = p;
                    while (*p++ != '\n');
                    ce = p;
                    if (value) {
                        ll = sl = kl = 0;
                        len = (ce - data) + strlen(ce);
                        goto insert;
                    } else {
                        for (ce--; ce != cb && (*(ce - 1) == ' ' || *(ce - 1) == '\t'); ce--);
                        if (!strNDup(&p, cb, ce - cb))
                            return 0;
                        return p;
                    }
                }
            }
            while (*p++ != '\n');
            if (insect)
                secinsert = p;
            else
                pastinsert = p;
        }
    }
    if (!value)
        return 0;
    len = p - data;
    if (secinsert) {
        ce = cb = secinsert;
        sl = ll = 0;
    } else {
        sl = strlen(section) + 3;
        if (pastinsert) {
            ce = cb = pastinsert;
            ll = 1;
        } else {
            ce = cb = data;
            ll = 0;
        }
    }
    kl = strlen(key) + 1;
  insert:
    vl = strlen(value);
    nlen = len - (ce - cb) + ll + sl + kl + vl + 1;
    if (!(p = ndata = Malloc(nlen + 1)))
        return data;
    apparr(p, data, cb - data); /* zero length if data is null */
    if (kl) {
        if (sl) {
            if (ll)
                appbyte(p, '\n');
            appbyte(p, '[');
            apparr(p, section, sl - 3);
            appbyte(p, ']');
            appbyte(p, '\n');
        }
        apparr(p, key, kl - 1);
        appbyte(p, '=');
    }
    apparr(p, value, vl);
    appbyte(p, '\n');
    if (data) {
        apparr(p, ce, len - (ce - data));
        free(data);
    }
    appbyte(p, 0);
    return ndata;
}

char *
iniMerge(char *data, const char *newdata)
{
    const char *p, *cb, *ce;
    char *section = 0, *key, *value;

    if (!newdata)
        return data;
    for (p = newdata;;) {
        for (; *p == ' ' || *p == '\t'; p++);
        if (!*p)
            break;
        if (*p == '\n') {
            p++;
            continue;
        }
        if (*p == '#') {
            for (p++; *p != '\n'; p++)
                if (!*p)
                    goto bail;
            p++;
            continue;
        }
        if (*p == '[') {
            cb = ++p;
            for (; *p != ']'; p++)
                if (!*p || *p == '\n') /* missing ] */
                    goto bail;
            if (!reStrN(&section, cb, p - cb))
                break;
            p++;
        } else {
            cb = p;
            for (; *p != '='; p++)
                if (!*p || *p == '\n') /* missing = */
                    goto bail;
            for (ce = p; ce != cb && (*(ce - 1) == ' ' || *(ce - 1) == '\t'); ce--);
            if (!strNDup(&key, cb, ce - cb))
                break;
            for (p++; *p == ' ' || *p == '\t'; p++);
            cb = p;
            for (; *p && *p != '\n'; p++);
            for (ce = p; ce != cb && (*(ce - 1) == ' ' || *(ce - 1) == '\t'); ce--);
            if (!strNDup(&value, cb, ce - cb))
                break;
            if (section)
                data = iniEntry(data, section, key, value);
            free(value);
            free(key);
        }
    }
  bail:
    free(section);
    return data;
}
