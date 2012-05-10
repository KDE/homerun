/************************************************************

Copyright 1998 by Thomas E. Dickey <dickey@clark.net>

   All Rights Reserved

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name(s) of the above copyright
holders shall not be used in advertising or otherwise to promote the
sale, use or other dealings in this Software without prior written
authorization.

********************************************************/


#ifndef _DM_ERROR_H_
#define _DM_ERROR_H_ 1

#include "greet.h"
#include <stdarg.h>

void gDebug(const char *fmt, ...);
void debug(const char *fmt, ...);
void logInfo(const char *fmt, ...);
void logWarn(const char *fmt, ...);
void logError(const char *fmt, ...);
void logPanic(const char *fmt, ...) ATTR_NORETURN;
void logOutOfMem(void);
void panic(const char *mesg) ATTR_NORETURN;
void initErrorLog(const char *errorLogFile);
#ifdef USE_SYSLOG
void reInitErrorLog(void);
#else
# define reInitErrorLog() while(0)
#endif
int ASPrintf(char **strp, const char *fmt, ...);
int VASPrintf(char **strp, const char *fmt, va_list args);

#endif /* _DM_ERROR_H_ */
