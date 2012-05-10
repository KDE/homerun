/****************************************************************************
**
** This file is based on sources of the Qt GUI Toolkit, used under the terms
** of the GNU General Public License version 2 (see the original copyright
** notice below).
** All further contributions to this file are (and are required to be)
** licensed under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** The original Qt license header follows:
**
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QWINDOWDEFS_H
#define QWINDOWDEFS_H

#include <stdlib.h>

#define Q_WS_X11

#define Q_EXPORT
#define QT_NO_DATASTREAM
#define QT_NO_IMAGE_TEXT
#define QT_NO_IMAGE_TRANSFORMATION
#define QT_NO_IMAGE_MIRROR
#define QT_NO_COMPONENT
#define QT_NO_IMAGEIO
#define QT_NO_MIME
#define QT_NO_QIMAGEIO
#define QT_NO_STRINGLIST
#define QT_NO_XFTFREETYPE
#ifndef QT_NO_STL
#define QT_NO_STL
#endif
#ifndef QT_NO_ASCII_CAST
#define QT_NO_ASCII_CAST
#endif
#ifndef QT_NO_CAST_ASCII
#define QT_NO_CAST_ASCII
#endif

#define QT_NO_IMAGEIO_BMP
#define QT_NO_IMAGEIO_PPM
#define QT_NO_IMAGEIO_XBM
#define QT_NO_IMAGEIO_XPM
#define QT_NO_IMAGEIO_MNG
#define QT_NO_ASYNC_IMAGE_IO

#define QT_STATIC_CONST static const
#define QT_STATIC_CONST_IMPL const

typedef int QCOORD;				// coordinate type
const QCOORD QCOORD_MAX =  2147483647;
const QCOORD QCOORD_MIN = -QCOORD_MAX - 1;

typedef unsigned int QRgb;			// RGB triplet

#define qMax(a, b)	((b) < (a) ? (a) : (b))
#define qMin(a, b)	((a) < (b) ? (a) : (b))
#define qAbs(a)	((a) >= 0  ? (a) : -(a))

#define Q_UNUSED(x) (void)x;

#define Q_CHECK_PTR(p)

#define Q_ASSERT(x)

typedef unsigned char   uchar;
typedef unsigned	uint;

typedef signed char		Q_INT8;		// 8 bit signed
typedef unsigned char		Q_UINT8;	// 8 bit unsigned
typedef short			Q_INT16;	// 16 bit signed
typedef unsigned short		Q_UINT16;	// 16 bit unsigned
typedef int			Q_INT32;	// 32 bit signed
typedef unsigned int		quint32;	// 32 bit unsigned
#if defined(Q_OS_WIN64)
typedef __int64			Q_LONG;		// word up to 64 bit signed
typedef unsigned __int64	Q_ULONG;	// word up to 64 bit unsigned
#else
typedef long			Q_LONG;		// word up to 64 bit signed
typedef unsigned long		Q_ULONG;	// word up to 64 bit unsigned
#endif


#if 0
const QRgb  RGB_MASK    = 0x00ffffff;		// masks RGB values

Q_EXPORT inline int qRed( QRgb rgb )		// get red part of RGB
{ return (int)((rgb >> 16) & 0xff); }

Q_EXPORT inline int qGreen( QRgb rgb )		// get green part of RGB
{ return (int)((rgb >> 8) & 0xff); }

Q_EXPORT inline int qBlue( QRgb rgb )		// get blue part of RGB
{ return (int)(rgb & 0xff); }

Q_EXPORT inline int qAlpha( QRgb rgb )		// get alpha part of RGBA
{ return (int)((rgb >> 24) & 0xff); }

Q_EXPORT inline QRgb qRgb( int r, int g, int b )// set RGB value
{ return (0xff << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff); }

Q_EXPORT inline QRgb qRgba( int r, int g, int b, int a )// set RGBA value
{ return ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff); }

Q_EXPORT inline int qGray( int r, int g, int b )// convert R,G,B to gray 0..255
{ return (r*11+g*16+b*5)/32; }

Q_EXPORT inline int qGray( QRgb rgb )		// convert RGB to gray 0..255
{ return qGray( qRed(rgb), qGreen(rgb), qBlue(rgb) ); }
#endif

inline
Q_EXPORT void qWarning( const char *, ... )	// print warning message
#if defined(Q_CC_GNU) && !defined(__INSURE__)
    __attribute__ ((format (printf, 1, 2)))
#endif
    {}

inline
Q_EXPORT void qFatal( const char *, ... )	// print fatal message and exit
#if defined(Q_CC_GNU)
    __attribute__ ((format (printf, 1, 2)))
#endif
    {
    abort();
    }

#include <stdio.h>

bool freadline( char* buf, int bufsize, FILE* datafile );
void strip_whitespace( char* line );
bool begins_with( const char* line, const char* str );

#endif
