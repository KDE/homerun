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

#include <config-workspace.h>
#include <config-X11.h>

#include "qnamespace.h"
#include "qcolor.h"
#include "x11_defs.h"
#include <vector>
#include <string.h>

#ifdef HAVE_XINERAMA
extern "C" { // for older XFree86 versions
#include <X11/extensions/Xinerama.h>
}
#endif

using namespace std;

Display* spl_dpy;
int spl_screen;
void* spl_visual;
int spl_depth;
Colormap spl_colormap;
int spl_cells;

static Display* appDpy;
static int appScreenCount;

static QRect totalScreenRect;
static vector<QRect> screenRects;
static int screens;

int screenCount()
    {
    return screens;
    }

QRect totalScreenGeometry()
    {
    return totalScreenRect;
    }

QRect screenGeometry(int screen)
    {
    if (screen < 0 || screen > screens - 1)
        {
            abort();
        }
    return screenRects[screen];
    }

void detectScreenGeometry()
    {
    Display* dpy = x11Display();
    totalScreenRect = QRect(0,
                            0,
                            WidthOfScreen( ScreenOfDisplay( dpy, DefaultScreen( dpy ))),
                            HeightOfScreen( ScreenOfDisplay( dpy, DefaultScreen( dpy ))));
#ifdef HAVE_XINERAMA
    // Xinerama code from Qt
    XineramaScreenInfo *xinerama_screeninfo = 0;
    int unused;
    bool use_xinerama = XineramaQueryExtension( dpy, &unused, &unused )
                        && XineramaIsActive( dpy );
    if (use_xinerama)
        {
        xinerama_screeninfo = XineramaQueryScreens( dpy, &screens );
        QRect sg;
        QRect total;

        for (int s = 0; s < screens; ++s)
            {
            QRect cs(xinerama_screeninfo[s].x_org,
                     xinerama_screeninfo[s].y_org,
                     xinerama_screeninfo[s].width,
                     xinerama_screeninfo[s].height);
            screenRects.push_back(cs);
            }
        }
    else
#endif
        {
        screens = 1;
        screenRects.push_back(totalScreenRect);
        }
    }

void createColormap()
    {
		    // use default colormap
		    XStandardColormap *stdcmap;
		    VisualID vid =
			XVisualIDFromVisual((Visual *) spl_visual );
		    int i, count;

		    spl_colormap = 0;
                    Display* appDpy = spl_dpy;
                    int screen = spl_screen;

		    if ( true /*|| ! serverVendor.contains( "Hewlett-Packard" )*/ ) {
			// on HPUX 10.20 local displays, the RGB_DEFAULT_MAP colormap
			// doesn't give us correct colors. Why this happens, I have
			// no clue, so we disable this for HPUX
			if (XGetRGBColormaps(appDpy,
					     RootWindow(spl_dpy,spl_screen)/*QPaintDevice::x11AppRootWindow( screen )*/,
					     &stdcmap, &count, XA_RGB_DEFAULT_MAP)) {
			    i = 0;
			    while (i < count &&
				   spl_colormap == 0) {
				if (stdcmap[i].visualid == vid) {
				    spl_colormap =
					stdcmap[i].colormap;
				}
				i++;
			    }

			    XFree( (char *)stdcmap );
			}
		    }

		    if (spl_colormap == 0) {
			spl_colormap =
			    DefaultColormap(appDpy, screen);
		    }
    }

static
int x11ErrorHandler(Display *d, XErrorEvent *e)
    {
    char msg[80], req[80], number[80];
    XGetErrorText(d, e->error_code, msg, sizeof(msg));
    sprintf(number, "%d", e->request_code);
    XGetErrorDatabaseText(d, "XRequest", number, "<unknown>", req, sizeof(req));

    fprintf(stderr, "%s(0x%lx): %s\n", req, e->resourceid, msg);
    return 0;
    }

bool openDisplay()
    {
    spl_dpy = XOpenDisplay( NULL );
    if( spl_dpy == NULL )
        return false;
    XSetErrorHandler( x11ErrorHandler );
    spl_screen = DefaultScreen( spl_dpy );
    spl_depth = DefaultDepth( spl_dpy, spl_screen );
    spl_visual = DefaultVisual( spl_dpy, spl_screen );
    spl_cells = DisplayCells( spl_dpy, spl_screen );
    appScreenCount = ScreenCount( spl_dpy );
    appDpy = spl_dpy;
    createColormap();
    detectScreenGeometry();
    QColor::initialize();
    return true;
    }

void closeDisplay()
    {
    XCloseDisplay( spl_dpy );
    }

static GC*	app_gc_ro	= 0;		// read-only GC
static GC*	app_gc_tmp	= 0;		// temporary GC
static GC*	app_gc_ro_m	= 0;		// read-only GC (monochrome)
static GC*	app_gc_tmp_m	= 0;		// temporary GC (monochrome)

static GC create_gc( int scrn, bool monochrome )
{
    GC gc;
    if ( monochrome ) {
	Pixmap pm = XCreatePixmap( appDpy, RootWindow( appDpy, scrn ), 8, 8, 1 );
	gc = XCreateGC( appDpy, pm, 0, 0 );
	XFreePixmap( appDpy, pm );
    } else {
	if ( QPaintDevice::x11AppDefaultVisual( scrn ) ) {
	    gc = XCreateGC( appDpy, RootWindow( appDpy, scrn ), 0, 0 );
	} else {
	    Window w;
	    XSetWindowAttributes a;
	    a.background_pixel = Qt::black.pixel( scrn );
	    a.border_pixel = Qt::black.pixel( scrn );
	    a.colormap = QPaintDevice::x11AppColormap( scrn );
	    w = XCreateWindow( appDpy, RootWindow( appDpy, scrn ), 0, 0, 100, 100,
			       0, QPaintDevice::x11AppDepth( scrn ), InputOutput,
			       (Visual*)QPaintDevice::x11AppVisual( scrn ),
			       CWBackPixel|CWBorderPixel|CWColormap, &a );
	    gc = XCreateGC( appDpy, w, 0, 0 );
	    XDestroyWindow( appDpy, w );
	}
    }
    XSetGraphicsExposures( appDpy, gc, False );
    return gc;
}

GC qt_xget_readonly_gc( int scrn, bool monochrome )	// get read-only GC
{
    if ( scrn < 0 || scrn >= appScreenCount ) {
#if 0
	qDebug("invalid screen %d %d", scrn, appScreenCount );
	QWidget* bla = 0;
	bla->setName("hello");
#endif
    }
    GC gc;
    if ( monochrome ) {
	if ( !app_gc_ro_m )			// create GC for bitmap
	    memset( (app_gc_ro_m = new GC[appScreenCount]), 0, appScreenCount * sizeof( GC ) );
	if ( !app_gc_ro_m[scrn] )
	    app_gc_ro_m[scrn] = create_gc( scrn, true );
	gc = app_gc_ro_m[scrn];
    } else {					// create standard GC
	if ( !app_gc_ro )
	    memset( (app_gc_ro = new GC[appScreenCount]), 0, appScreenCount * sizeof( GC ) );
	if ( !app_gc_ro[scrn] )
	    app_gc_ro[scrn] = create_gc( scrn, false );
	gc = app_gc_ro[scrn];
    }
    return gc;
}

GC qt_xget_temp_gc( int scrn, bool monochrome )		// get temporary GC
{
    if ( scrn < 0 || scrn >= appScreenCount ) {
#if 0
	qDebug("invalid screen (tmp) %d %d", scrn, appScreenCount );
	QWidget* bla = 0;
	bla->setName("hello");
#endif
    }
    GC gc;
    if ( monochrome ) {
	if ( !app_gc_tmp_m )			// create GC for bitmap
	    memset( (app_gc_tmp_m = new GC[appScreenCount]), 0, appScreenCount * sizeof( GC ) );
	if ( !app_gc_tmp_m[scrn] )
	    app_gc_tmp_m[scrn] = create_gc( scrn, true );
	gc = app_gc_tmp_m[scrn];
    } else {					// create standard GC
	if ( !app_gc_tmp )
	    memset( (app_gc_tmp = new GC[appScreenCount]), 0, appScreenCount * sizeof( GC ) );
	if ( !app_gc_tmp[scrn] )
	    app_gc_tmp[scrn] = create_gc( scrn, false );
	gc = app_gc_tmp[scrn];
    }
    return gc;
}
