/********************************************************************

Copyright (C) 2007 Lubos Lunak <l.lunak@kde.org>

Please see file LICENSE for the licensing terms of ksplashx as a whole.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

//#define DEBUG

const int MAX_ITEMS = 100;
const int ANIM_IMAGES_ROW = 10;

// for converting from startup states to (internal) numbers
// these are also in the simple splash and in krunner
const char states[][ 12 ] =
    { "initial", "kded", "confupdate", "kcminit", "ksmserver", "wm", "desktop", "ready" };
// State "ready" isn't used, make splash go away as soon as desktop is ready.
const int LAST_STATE = 6;

extern int screen_number;

#include <config-workspace.h>

#include "splash.h"
#include "qcolor.h"
#include "qimage.h"
#include "pixmap.h"
#include "scale.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <libgen.h>
#include <math.h>

#include <X11/Xutil.h>

# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# endif
#include <time.h>

struct AnimData
    {
    AnimData( int x, int y, PixmapData* frames, int num_frames, int delay, int repeat );
    ~AnimData();
    bool updateFrame( int change );
    int x, y;
    PixmapData* frames;
    int num_frames;
    int current_frame;
    int delay;
    int remaining_delay;
    int repeat;
    int remaining_repeat;
    };

AnimData::AnimData( int x, int y, PixmapData* frames, int num_frames, int delay, int repeat )
    : x( x )
    , y( y )
    , frames( frames )
    , num_frames( num_frames )
    , current_frame( 0 )
    , delay( delay )
    , remaining_delay( delay )
    , repeat( repeat )
    , remaining_repeat( repeat )
    {
    }

AnimData::~AnimData()
    {
    for( int i = 0;
         i < num_frames;
         ++i )
        {
        if( frames[ i ].hd != None )
            XFreePixmap( qt_xdisplay(), frames[ i ].hd );
        }
    delete[] frames;
    }

bool AnimData::updateFrame( int change )
    {
    remaining_delay -= change;
    bool ret = false;
    while( remaining_delay <= 0 )
        {
        if( ++current_frame == num_frames )
            {
            if( repeat > 0 && --remaining_repeat <= 0 ) // non-infinite and finished
                { // stay at the last frame
                --current_frame;
                remaining_delay = 100000000;
                return false;
                }
            current_frame = 0;
            }
        remaining_delay += delay;
        ret = true;
        }
    return ret;
    }

static QImage splash_image; // contents of the splash window (needed for alphablending)
static Pixmap splash_pixmap; // the pixmap with window contents
static AnimData* animations[ MAX_ITEMS ];
static int anim_count;
static Window window = None;
static QRect geometry;
static bool scalex = true;
static bool scaley = true;
static Atom kde_splash_progress;
static char kdehome[ 1024 ];
static char theme_name[ 1024 ];
static char theme_dir[ 1024 ];
static bool test;
static int parent_pipe;
static time_t final_time;
static int state;
static time_t timestamp; // timestamp of the description.txt file, used for caching

// returns a pointer to a static !
static const char* findFileHelper( const char* name, int* w, int* h, bool locolor, bool lame, QRect geom )
    {
    static char tmp[ 1024 ];
    char best[ 1024 ];
    int best_w = -1;
    int best_h = -1;
    float best_distance = 10E30;
    DIR* dir = opendir( theme_dir );
    if( dir != NULL )
        {
        while( dirent* file = readdir( dir ))
            {
            int w, h;
            if( locolor
                ? sscanf( file->d_name, "%dx%d-locolor", &w, &h ) == 2
                : sscanf( file->d_name, "%dx%d", &w, &h ) == 2 )
                {
                // compute difference of areas
                float delta = w * h - geom.width() * geom.height();
                // scale down to about 1.0
                delta /= ((geom.width() * geom.height())+(w * h))/2;
                // Consider first the difference in aspect ratio,
                // then in areas. Prefer scaling down.
                float deltaRatio = 1.0;
                if (h > 0 && geom.height() > 0) {
                    deltaRatio = float(w) / float(h) -
                                 float(geom.width()) / float(geom.height());
                }
                float distance = fabs(deltaRatio) * 3.0 + (delta >= 0.0 ? delta : -delta + 5.0);
                if( distance < best_distance
                    // only derive from themes with the same ratio if lame resolutions are not allowed, damn 1280x1024
                    && ( lame || w * geom.height() == h * geom.width())
                    )
                    {
                    snprintf( tmp, 1024, "%s/%dx%d%s/%s", theme_dir, w, h, locolor ? "-locolor" : "", name );
#ifdef DEBUG
                    fprintf( stderr, "FINDFILE3: %s %s\n", name, tmp );
#endif
                    if( access( tmp, R_OK ) == 0 )
                        {
                        best_w = w;
                        best_h = h;
                        best_distance = distance;
                        strcpy( best, tmp );
                        }
                    }
                }
            }
        closedir( dir );
        }
    if( best_w > 0 )
        {
        if( w != NULL )
            *w = best_w;
        if( h != NULL )
            *h = best_h;
        strcpy( tmp, best );
        return tmp;
        }
    return "";
    }

// returns a pointer to a static !
static const char* findFileWithDepth( const char* name, int* w, int* h, bool locolor, QRect geom )
    {
    static char tmp[ 1024 ];
    snprintf( tmp, 1024, "%s/%dx%d%s/%s", theme_dir, geom.width(), geom.height(),
        locolor ? "-locolor" : "", name );
#ifdef DEBUG
    fprintf( stderr, "FINDFILE1: %s %s\n", name, tmp );
#endif
    if( access( tmp, R_OK ) != 0 )
        {
        // ksplash/<theme>-<resolution>-<file> in 'kde-config --path cache'
        static char hostname[ 1024 ];
        if( getenv("XAUTHLOCALHOSTNAME"))
            strncpy( hostname, getenv("XAUTHLOCALHOSTNAME"), 1023 );
        else
            gethostname( hostname, 1023 );
        hostname[ 1023 ] = '\0';
        snprintf( tmp, 1024, "%s/cache-%s/ksplashx/%s-%dx%d%s-%s", kdehome, hostname, theme_name,
            geom.width(), geom.height(), locolor ? "-locolor" : "", name );
#ifdef DEBUG
        fprintf( stderr, "FINDFILE2: %s %s\n", name, tmp );
#endif
        struct stat stat_buf;
        if( stat( tmp, &stat_buf ) != 0 || stat_buf.st_mtime != timestamp )
            {
            tmp[ 0 ] = '\0';
#ifdef DEBUG
            fprintf( stderr, "FINDFILE2 TIMESTAMP FAILURE\n" );
#endif
            }
        }
    if( access( tmp, R_OK ) == 0 )
        {
        if( w != NULL )
            *w = geom.width();
        if( h != NULL )
            *h = geom.height();
        return tmp;
        }
    if( w == NULL || h == NULL ) // no scaling possible
        return "";
#if 0 // disable for now in order to match plasma's selection mechanism
    const char* ret = findFileHelper( name, w, h, locolor, false, geom );
    if( ret == NULL || *ret == '\0' )
        ret = findFileHelper( name, w, h, locolor, true, geom );
    return ret;
#else
    return findFileHelper( name, w, h, locolor, true, geom );
#endif
    }

// returns a pointer to a static !
static const char* findLocalizedFileWithDepth( const char* name, int* w, int* h, bool locolor, QRect geom )
    {
    const int bufsz = 1024;

    // Split name into dirname and basename.
    char name2[ bufsz ];
    strncpy( name2, name, bufsz );
    name2[ bufsz - 1 ] = '\0';
    char* basn = basename( name2 ); // must preceed dirname
    char* dirn = dirname( name2 ); // modifies name2

    // Check for localized file by parsing languages from KLOCALE_LANGUAGES,
    // as provided by startkde via kstartupconfig4. It contains list of
    // language codes, colon-separated and ordered by decreasing priority.
    const char* lvarname = "KLOCALE_LANGUAGES";
    if( getenv( lvarname ) && getenv( lvarname )[ 0 ] )
        {
        char lvar[ bufsz ];
        strncpy( lvar, getenv( lvarname ), bufsz );
        lvar[ bufsz - 1 ] = '\0';

        // Go through colon-separated list of languages.
        char* lang = strtok( lvar, ":" );
        while( 1 )
            {
            char locname[ bufsz ];
            snprintf( locname, bufsz, "%s/l10n/%s/%s", dirn, lang, basn );
            locname[ bufsz - 1 ] = '\0';

            // Check if this path exists.
            const char* path = findFileWithDepth( locname, w, h, locolor, geom );
            if( path[ 0 ] )
                return path;

            if( ( lang = strtok( 0, ":" ) ) == 0 )
                break;
            }
        }

    // Fall back to unlocalized file.
    return findFileWithDepth( name, w, h, locolor, geom );
    }

// returns a pointer to a static !
static const char* findFile( const char* name, int* w = NULL, int* h = NULL, bool* locolor = NULL, QRect geom = screenGeometry(0) )
    {
    if( x11Depth() <= 8 )
        {
        if( const char* ret = findLocalizedFileWithDepth( name, w, h, true, geom )) // try locolor
            {
            if( locolor != NULL )
                *locolor = true;
            return ret;
            }
        }
    if( locolor != NULL )
        *locolor = false;
    return findLocalizedFileWithDepth( name, w, h, false, geom); // no locolor
    }

// If a properly sized image doesn't exist save it in the cache location
// for the next use, because that means no scaling and a smaller png image
// to load.
static void pregeneratePixmap( const char* file, const char* real_file, int width, int height, bool locolor, QRect geom )
    {
#ifdef DEBUG
    static char cmd[ 1024 ];
    snprintf( cmd, 1024, "ksplashx_scale \"%s\" \"%s\" \"%s\" %d %d %d %d %ld %s", theme_name,
        file, real_file, width, height, geom.width(), geom.height(), timestamp,
        locolor ? "locolor" : "no-locolor" );
    fprintf( stderr, "PREGENERATE PIXMAP CMD:%s\n", cmd );
#endif
    char w[ 20 ], h[ 20 ], sw[ 20 ], sh[ 20 ], t[ 40 ];
    sprintf( w, "%d", width );
    sprintf( h, "%d", height );
    sprintf( sw, "%d", geom.width());
    sprintf( sh, "%d", geom.height());
    sprintf( t, "%ld", timestamp );
    if( fork() == 0 )
        {
        int maxf = sysconf( _SC_OPEN_MAX );
        for( int f = 0;
             f < maxf;
             ++f )
            close( f );
        nice( 10 );
        sleep( 30 );
        char* args[ 20 ];
        args[ 0 ] = const_cast< char* >( "ksplashx_scale" );
        args[ 1 ] = theme_name;
        args[ 2 ] = ( char* ) file;
        args[ 3 ] = ( char* ) real_file;
        args[ 4 ] = w;
        args[ 5 ] = h;
        args[ 6 ] = sw;
        args[ 7 ] = sh;
        args[ 8 ] = t;
        args[ 9 ] = ( char* )( locolor ? "locolor" : "no-locolor" );
        args[ 10 ] = NULL;
        execvp( args[ 0 ], args );
        _exit( 0 );
        }
    }

static QImage readImage( FILE* f )
    {
    const char jpeg[ 3 + 1 ] = "\377\330\377";
    char buf[ 3 ] = "";
    fread( buf, 1, 3, f );
    rewind( f );
    if( memcmp( buf, jpeg, 3 ) == 0 )
        return splash_read_jpeg_image( f );
    return splash_read_png_image( f );
    }

static QImage loadImage( const char* file, QRect geom )
    {
    int w, h;
    bool locolor;
    const char* real_file = findFile( file, &w, &h, &locolor, geom ); // points to a static !
    FILE* f = fopen( real_file, "r" );
    if( f == NULL )
        return QImage();
    QImage img = readImage( f );
    if( img.depth() != 32 )
        img = img.convertDepth( 32 );
    fclose( f );
    if( img.isNull())
        {
        fprintf( stderr, "Failed to load: %s\n", file );
        exit( 3 );
        }
    if( img.depth() != 32 )
        {
        fprintf( stderr, "Not 32bpp: %s\n", file );
        exit( 3 );
        }
    if(( scalex && w != geom.width()) || ( scaley && h != geom.height()))
        {
        double ratiox = scalex ? double( w ) / geom.width() : 1;
        double ratioy = scaley ? double( h ) / geom.height() : 1;
#ifdef DEBUG
        fprintf( stderr, "PIXMAP SCALING: %f %f\n", ratiox, ratioy );
#endif
        img = scale( img, round( img.width() / ratiox ), round( img.height() / ratioy ));
        if( ratiox * ratioy > 1 ) // only downscale
            pregeneratePixmap( file, real_file, img.width(), img.height(), locolor, geom );
        }
    return img;
    }

static void frameSize( const QImage& img, int frames, int& framew, int& frameh )
    {
    if( frames < ANIM_IMAGES_ROW )
        {
        framew = img.width() / frames;
        frameh = img.height();
        }
    else
        {
        framew = img.width() / ANIM_IMAGES_ROW;
        frameh = img.height() / (( frames + ANIM_IMAGES_ROW - 1 ) / ANIM_IMAGES_ROW );
        }
    }

static QImage loadAnimImage( const char* file, int frames )
    {
    int w, h;
    bool locolor;
    const char* real_file = findFile( file, &w, &h, &locolor ); // points to a static !
    FILE* f = fopen( real_file, "r" );
    if( f == NULL )
        {
        fprintf( stderr, "Bad anim file: %s\n", file );
        exit( 3 );
        }
    QImage img = readImage( f );
    if( img.depth() != 32 )
        img = img.convertDepth( 32 );
    fclose( f );
    int framew, frameh;
    if( frames < ANIM_IMAGES_ROW )
        {
        if( img.width() % frames != 0 )
            {
            fprintf( stderr, "Bad anim size: %s\n", file );
            exit( 3 );
            }
        }
    else
        {
        if( img.width() % ANIM_IMAGES_ROW != 0
            || img.height() % (( frames + ANIM_IMAGES_ROW - 1 ) / ANIM_IMAGES_ROW ) != 0 )
            {
            fprintf( stderr, "Bad anim size: %s\n", file );
            exit( 3 );
            }
        }
    frameSize( img, frames, framew, frameh );
    if(( scalex && w != screenGeometry(0).width()) || ( scaley && h != screenGeometry(0).height()))
        {
        double ratiox = scalex ? double( w ) / screenGeometry(0).width() : 1;
        double ratioy = scaley ? double( h ) / screenGeometry(0).height() : 1;
#ifdef DEBUG
        fprintf( stderr, "ANIM SCALING: %f %f\n", ratiox, ratioy );
#endif
        int framewnew = round( framew / ratiox );
        int framehnew = round( frameh / ratioy );
        QImage imgnew( framewnew * qMin( frames, ANIM_IMAGES_ROW ),
            framehnew * (( frames + ANIM_IMAGES_ROW - 1 ) / ANIM_IMAGES_ROW ), img.depth());
        if( img.hasAlphaBuffer())
            imgnew.setAlphaBuffer( true );
        for( int frame = 0;
             frame < frames;
             ++frame )
            {
            QImage im2 = img.copy( ( frame % ANIM_IMAGES_ROW ) * framew, ( frame / ANIM_IMAGES_ROW ) * frameh, framew, frameh );
            im2 = scale( im2, framewnew, framehnew );
            // don't use bitBlt(), it'd apply also alpha
            for( int y = 0;
                 y < im2.height();
                 ++y )
                {
                QRgb* s = ( QRgb* ) im2.scanLine( y );
                QRgb* d = (( QRgb* ) imgnew.scanLine( y + ( frame / ANIM_IMAGES_ROW ) * framehnew ))
                    + ( frame % ANIM_IMAGES_ROW ) * framewnew;
                memcpy( d, s, im2.width() * sizeof( QRgb ));
                }
            }
        framew = framewnew;
        frameh = framehnew;
        img = imgnew;
        if( ratiox * ratioy > 1 ) // only downscale
            pregeneratePixmap( file, real_file, img.width(), img.height(), locolor, screenGeometry(0) );
        }
    return img;
    }

static PixmapData* imageAnimToPixmaps( const QImage& img, int frames )
    {
    if( img.isNull())
        return NULL;
    int framew, frameh;
    frameSize( img, frames, framew, frameh );
    PixmapData pix = imageToPixmap( img );
    PixmapData* ret = new PixmapData[ MAX_ITEMS ];
    GC gc = qt_xget_temp_gc( x11Screen(), false );
    for( int frame = 0;
         frame < frames;
         ++frame )
        {
        Pixmap p = XCreatePixmap( qt_xdisplay(), DefaultRootWindow( qt_xdisplay()), framew, frameh, x11Depth());
        XCopyArea( qt_xdisplay(), pix.hd, p, gc,
            ( frame % ANIM_IMAGES_ROW ) * framew, ( frame / ANIM_IMAGES_ROW ) * frameh, framew, frameh, 0, 0 );
        ret[ frame ].hd = p;
        ret[ frame ].w = framew;
        ret[ frame ].h = frameh;
        ret[ frame ].d = x11Depth();
        }
    if( pix.hd != None )
        XFreePixmap( qt_xdisplay(), pix.hd );
    return ret;
    }

static void doPaint( const QRect& area )
    {
#if 0
    fprintf( stderr, "PAINT: %d,%d-%dx%d\n", area.x(), area.y(), area.width(), area.height());
#endif
    if( window == None )
        return; // delayed
    // double-buffer
    Pixmap pixmap = XCreatePixmap( qt_xdisplay(), DefaultRootWindow( qt_xdisplay()),
        area.width(), area.height(), x11Depth());
    GC gc = qt_xget_temp_gc( x11Screen(), false );
    // copy splash pixmap
    XCopyArea( qt_xdisplay(), splash_pixmap, pixmap, gc,
        area.x(), area.y(), area.width(), area.height(), 0, 0 );
    // add animations
    for( int i = 0;
         i < MAX_ITEMS;
         ++i )
        {
        AnimData* anim = animations[ i ];
        PixmapData* frame = anim != NULL ? &anim->frames[ anim->current_frame ] : NULL;
        if( anim != NULL
            && area.intersects( QRect( anim->x, anim->y, frame->w, frame->h )))
            {
            XCopyArea( qt_xdisplay(), frame->hd, pixmap, gc,
                qMax( 0, area.x() - anim->x ), qMax( 0, area.y() - anim->y ),
                area.x() - anim->x + area.width(), area.y() - anim->y + area.height(),
                qMax( 0, anim->x - area.x()), qMax( 0, anim->y - area.y()));
            }
        }
    XCopyArea( qt_xdisplay(), pixmap, window, gc, 0, 0, area.width(), area.height(), area.x(), area.y());
    XFreePixmap( qt_xdisplay(), pixmap );
    }

static void createWindow()
    {
    assert( window == None );
#ifdef DEBUG
    fprintf( stderr, "GEOMETRY: %d %d %d %d\n", geometry.x(), geometry.y(), geometry.width(), geometry.height());
#endif
    XSetWindowAttributes attrs;
    QRect geom = totalScreenGeometry();
    attrs.override_redirect = True;
    attrs.background_pixmap = None;
//    attrs.override_redirect = False;
    window = XCreateWindow( qt_xdisplay(), DefaultRootWindow( qt_xdisplay()),
        geom.x(), geom.y(), geom.width(), geom.height(),
        0, CopyFromParent, CopyFromParent, CopyFromParent, CWOverrideRedirect | CWBackPixmap, &attrs );
    XSelectInput( qt_xdisplay(), window, ButtonPressMask | ExposureMask );
    XClassHint class_hint;
    class_hint.res_name = const_cast< char* >( "ksplashx" );
    class_hint.res_class = const_cast< char* >( "ksplashx" );
    XSetWMProperties( qt_xdisplay(), window, NULL, NULL, NULL, 0, NULL, NULL, &class_hint );
    XMapRaised( qt_xdisplay(), window );
    }

static void createSplashImage()
    {
    QRect geom = totalScreenGeometry();
    assert( splash_image.isNull());
    assert( splash_pixmap == None );
    splash_image = QImage( geom.size(), 32 );
    splash_pixmap = XCreatePixmap( qt_xdisplay(), DefaultRootWindow( qt_xdisplay()),
        geom.width(), geom.height(), x11Depth());
    }

static bool waitState( int expected_state )
    {
    if( expected_state <= state )
        return false;
    if( window == None )
        createWindow();
    if( splash_image.isNull())
        {
        fprintf( stderr, "No window contents\n" );
        exit( 3 );
        }
    time_t test_time = time( NULL ) + 2;
#ifdef DEBUG
    fprintf( stderr,"AWATING STATE: %d (%s)\n", expected_state, states[ expected_state ] );
#endif
    if( parent_pipe >= 0 && screen_number == 0)
        { // wait for paint being finished, and tell parent to exit
        XSync( qt_xdisplay(), False );
        char buf = '\0';
        write( parent_pipe, &buf, 1 );
        close( parent_pipe );
        parent_pipe = -1;
        }
    const int doubleclick_delay = 200; // mouse doubleclick delay - in ms
    struct timeval button_press_time, current_time; // we need timeval to deal with milliseconds
    button_press_time.tv_sec = 0;
    button_press_time.tv_usec = 0;
    long click_delay, click_delay_seconds, click_delay_useconds;
    for(;;)
        {
        while( XPending( qt_xdisplay()))
            {
            XEvent ev;
            XNextEvent( qt_xdisplay(), &ev );
            if( ev.type == ButtonPress && ev.xbutton.window == window && ev.xbutton.button == Button1 )
                {
                gettimeofday( &current_time, NULL );
                
                // find difference in milliseconds with current and previous mouse presses times
                click_delay_seconds  = current_time.tv_sec  - button_press_time.tv_sec;
                click_delay_useconds = current_time.tv_usec - button_press_time.tv_usec;
                click_delay = ( click_delay_seconds * 1000 + click_delay_useconds / 1000.0 ) + 0.5;
            
                if( click_delay <= doubleclick_delay )
                    {
                    // close splash on doubleclick
                    final_time = time( NULL );
                    break;
                    }
                gettimeofday( &button_press_time, NULL );
                }
            if( ev.type == Expose && ev.xexpose.window == window )
                doPaint( QRect( ev.xexpose.x, ev.xexpose.y, ev.xexpose.width, ev.xexpose.height ));
            if( ev.type == ConfigureNotify && ev.xconfigure.event == DefaultRootWindow( qt_xdisplay()))
                XRaiseWindow( qt_xdisplay(), window );
            if( ev.type == ClientMessage && ev.xclient.window == DefaultRootWindow( qt_xdisplay())
                && ev.xclient.message_type == kde_splash_progress )
                {
                // based on ksplash
                const char* s = ev.xclient.data.b;
#ifdef DEBUG
                fprintf( stderr,"MESSAGE: %s\n", s );
#endif
                int new_state = -1;
                for( int i = 0;
                     i < int( sizeof( states ) / sizeof( states[ 0 ] ));
                     ++i )
                    {
                    if( strcmp( s, states[ i ] ) == 0 )
                        {
                        new_state = i;
                        break;
                        }
                    }
                if( new_state == -1 )
                    {
#ifdef DEBUG
                    fprintf( stderr, "UNKNOWN SPLASH STATE: %s\n", s );
#endif
                    }
                else if( new_state > state )
                    {
                    state = new_state;
                    if( state >= LAST_STATE )
                        final_time = time( NULL ) + 1; // quit after short time
                    }
                }
            }
        if( test && time( NULL ) >= test_time )
            {
            ++state;
            test_time = time( NULL ) + 2;
            }
        if( expected_state <= state )
            return false;
        struct timeval tm_start, tm_end;
        gettimeofday( &tm_start, NULL );
        fd_set set;
        FD_ZERO( &set );
        FD_SET( XConnectionNumber( qt_xdisplay()), &set );
        int delay = 1000;
        for( int i = 0;
             i < MAX_ITEMS;
             ++i )
            if( animations[ i ] != NULL && animations[ i ]->delay < delay * 2 )
                delay = animations[ i ]->delay / 2;
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = delay * 1000;
        select( XConnectionNumber( qt_xdisplay()) + 1, &set, NULL, NULL, &tv );
        if( time( NULL ) >= final_time )
            {
#ifdef DEBUG
            fprintf( stderr, "EXITING\n" );
#endif
            return true; // --->
            }
        gettimeofday( &tm_end, NULL );
        int real_delay = (( tm_end.tv_sec - tm_start.tv_sec ) * 1000000 + tm_end.tv_usec - tm_start.tv_usec ) / 1000;
        for( int i = 0;
             i < MAX_ITEMS;
             ++i )
            {
            AnimData* anim = animations[ i ];
            if( anim != NULL && anim->updateFrame( real_delay ))
                doPaint( QRect( anim->x, anim->y, anim->frames[ anim->current_frame ].w, anim->frames[ anim->current_frame ].h ));
            }
        }
    }

static bool checkRelative( const char* ref )
    {
    if( ref[ 0 ] == '\0' || ref[ 1 ] == '\0' || ref[ 2 ] != '\0' )
        return false;
    if( strchr( "LRC", ref[ 0 ] ) == NULL )
        return false;
    if( strchr( "TBC", ref[ 1 ] ) == NULL )
        return false;
    return true;
    }

static int makeAbsolute( char screen, int val, char image, int size, int screen_size )
    {
    int pos;
    switch( screen )
        {
        case 'L':
        case 'T':
            pos = 0;
          break;
        case 'R':
        case 'B':
            pos = screen_size;
          break;
        case 'C':
            pos = screen_size / 2;
          break;
        default:
            exit( 3 );
        }
    pos += val;
    switch( image )
        {
        case 'L':
        case 'T':
            pos -= 0;
          break;
        case 'R':
        case 'B':
            pos -= size;
          break;
        case 'C':
            pos -= size / 2;
          break;
        default:
            exit( 3 );
        }
    return pos;
    }

static int makeAbsoluteX( const char* screen_ref, int x_rel, const char* image_ref, int width )
    {
    return makeAbsolute( screen_ref[ 0 ], x_rel, image_ref[ 0 ], width, geometry.width()) + geometry.x();
    }

static int makeAbsoluteY( const char* screen_ref, int y_rel, const char* image_ref, int height )
    {
    return makeAbsolute( screen_ref[ 1 ], y_rel, image_ref[ 1 ], height, geometry.height()) + geometry.y();
    }

static inline QRgb blend( QRgb c, QRgb background )
    {
    if( qAlpha( c ) == 255 )
        return c;
    return qRgb( ( qRed( background ) * ( 255 - qAlpha( c ) ) + qRed( c ) * qAlpha( c ) ) / 255,
                 ( qGreen( background ) * ( 255 - qAlpha( c ) ) + qGreen( c ) * qAlpha( c ) ) / 255,
                 ( qBlue( background ) * ( 255 - qAlpha( c ) ) + qBlue( c ) * qAlpha( c ) ) / 255 );
    }

static void blend( QImage& img, int x_pos, int y_pos, int x_img, int y_img, int w_img, int h_img )
    {
    if( !img.hasAlphaBuffer())
        return; // it doesn't have alpha, so it is the blended result
    for( int y = 0;
         y < h_img;
         ++y )
        {
        QRgb* s = (( QRgb* )( splash_image.scanLine( y + y_pos ))) + x_pos;
        QRgb* d = (( QRgb* )( img.scanLine( y + y_img ))) + x_img;
        for( int x = 0;
             x < w_img;
             ++x, ++d, ++s )
            {
            *d = blend( *d, *s );
            }
        }
    }

static void blend( QImage& img, int x_pos, int y_pos )
    {
    blend( img, x_pos, y_pos, 0, 0, img.width(), img.height());
    img.setAlphaBuffer( false );
    }

static void blendAnim( QImage& img, int x_pos, int y_pos, int frames )
    {
    int framew, frameh;
    frameSize( img, frames, framew, frameh );
    for( int frame = 0;
         frame < frames;
         ++frame )
        {
        blend( img, x_pos, y_pos,
            ( frame % ANIM_IMAGES_ROW ) * framew, ( frame / ANIM_IMAGES_ROW ) * frameh, framew, frameh );
        }
    img.setAlphaBuffer( false );
    }

static void updateSplashImage( const QImage& img, int x_pos, int y_pos )
    {
    for( int y = 0;
         y < img.height();
         ++y )
        {
        QRgb* s = (( QRgb* )( img.scanLine( y )));
        QRgb* d = (( QRgb* )( splash_image.scanLine( y + y_pos ))) + x_pos;
        for( int x = 0;
             x < img.width();
             ++x, ++d, ++s )
            {
            *d = *s;
            }
        }
    PixmapData pix = imageToPixmap( img );
    GC gc = qt_xget_temp_gc( x11Screen(), false );
    XCopyArea( qt_xdisplay(), pix.hd, splash_pixmap, gc, 0, 0, img.width(), img.height(), x_pos, y_pos );
    XFreePixmap( qt_xdisplay(), pix.hd );
    }

void runSplash( const char* them, bool t, int p )
    {
    geometry = screenGeometry(0);
    // fetch the $KDEHOME environment variable that may point to e.g. "~/.kde4"
    if( getenv( "KDEHOME" ) && getenv( "KDEHOME" )[ 0 ] )
        snprintf( kdehome, 1024, "%s", getenv( "KDEHOME" ));
    else
        snprintf( kdehome, 1024, "%s/"KDE_DEFAULT_HOME, getenv( "HOME" ) ? getenv( "HOME" ) : "" );

    // fetch the name of the theme which is also used as directory name.
    snprintf( theme_name, 1024, "%s", them );
    // fetch the theme-directory,e.g. "/opt/kde4/share/apps/ksplash/Themes/MyKSplashXThemeName"
    snprintf( theme_dir, 1024, "%s/ksplash/Themes/%s", KDE_DATADIR, them );

    test = t;
    parent_pipe = p;
    anim_count = 0;
    state = 0;
    window = None;
    splash_image = QImage();
    splash_pixmap = None;
    final_time = time( NULL ) + 300;
    int desc_w, desc_h;

    // try to load the themes description.txt file from within the theme_dir
    FILE* datafile = fopen( findFile( "description.txt", &desc_w, &desc_h ), "r" );
    if( datafile == NULL )
        {
            // if we failed to read it, try it with $KDEHOME as theme_dir. This
            // is needed to be able to load local (aka by the user in his local
            // home-directory) installed themes.
            snprintf( theme_dir, 1024, "%s/share/apps/ksplash/Themes/%s", kdehome, them );
            datafile = fopen( findFile( "description.txt", &desc_w, &desc_h ), "r" );
            if( datafile == NULL )
                {
                fprintf( stderr, "Cannot find description.txt file.\n" );
                exit( 2 );
                }
        }

    struct stat stat_buf;
    if( fstat( fileno( datafile ), &stat_buf ) != 0 )
        {
        fprintf( stderr, "Cannot read description.txt file.\n" );
        exit( 2 );
        }
    timestamp = stat_buf.st_mtime;
    // check also Theme.rc, as artists are likely to just ignore updating description.txt
    // when updating the theme but will touch info in Theme.rc
    char tmp[ 1024 ];
    snprintf( tmp, 1024, "%s/Theme.rc", theme_dir );
    if( stat( tmp, &stat_buf ) == 0 && stat_buf.st_mtime > timestamp )
	timestamp = stat_buf.st_mtime;
    double ratiox = double( desc_w ) / screenGeometry(0).width(); // only for coordinates in the description file
    double ratioy = double( desc_h ) / screenGeometry(0).height(); // only for coordinates in the description file
    XSelectInput( qt_xdisplay(), DefaultRootWindow( qt_xdisplay()), SubstructureNotifyMask );
    kde_splash_progress = XInternAtom( qt_xdisplay(), "_KDE_SPLASH_PROGRESS", False );
    for( int i = 0;
         i < MAX_ITEMS;
         ++i )
        animations[ i ] = NULL;
    while( !feof( datafile ))
        {
        char line[ 1024 ];
        if( !freadline( line, 1024, datafile ))
            break;
        strip_whitespace( line );
        char buf[ 1024 ];
        int number, x, y, w, h, x_rel, y_rel, frames, delay, repeat, items;
        char screen_ref[ 3 ];
        char window_ref[ 3 ];
        char image_ref[ 3 ];
        bool handled = false;
        if( line[ 0 ] == '#' || line[ 0 ] == '\0' )
            continue;
        else if( sscanf( line, "SCALEX %1023s", buf ) == 1 )
            {
            handled = true;
            if( strcmp( buf, "ON" ) == 0 )
                scalex = true;
            else if( strcmp( buf, "OFF" ) == 0 )
                scalex = false;
            else
                {
                fprintf( stderr, "Bad scale x: %s\n", line );
                exit( 3 );
                }
            }
        else if( sscanf( line, "SCALEY %1023s", buf ) == 1 )
            {
            handled = true;
            if( strcmp( buf, "ON" ) == 0 )
                scaley = true;
            else if( strcmp( buf, "OFF" ) == 0 )
                scaley = false;
            else
                {
                fprintf( stderr, "Bad scale y: %s\n", line );
                exit( 3 );
                }
            }
        else if( sscanf( line, "SCALE %1023s", buf ) == 1 )
            {
            handled = true;
            if( strcmp( buf, "ON" ) == 0 )
                scalex = scaley = true;
            else if( strcmp( buf, "OFF" ) == 0 )
                scalex = scaley = false;
            else
                {
                fprintf( stderr, "Bad scale: %s\n", line );
                exit( 3 );
                }
            }
        else if( sscanf( line, "GEOMETRY %d %d %d %d", &x, &y, &w, &h ) == 4 )
            {
            handled = true;
            if( scalex || scaley )
                {
                x = scalex ? round( x / ratiox ) : x;
                y = scaley ? round( y / ratioy ) : y;
                w = scalex ? round( w / ratiox ) : w;
                h = scaley ? round( h / ratioy ) : h;
                }
            if( x < 0 )
                x += screenGeometry(0).width();
            if( y < 0 )
                y += screenGeometry(0).height();
            QRect r( x, y, w, h );
            if( screenGeometry(0).contains( r ))
                {
                geometry = r;
                if( window != None )
                    XMoveResizeWindow( qt_xdisplay(), window, x, y, w, h );
                if( !splash_image.isNull())
                    { // destroy and then recreate
                    splash_image = QImage();
                    XFreePixmap( qt_xdisplay(), splash_pixmap );
                    splash_pixmap = None;
                    }
                createSplashImage();
                }
            else
                {
                fprintf( stderr, "Wrong geometry: %s\n", line );
                exit( 3 );
                }
            }
        else if( sscanf( line, "GEOMETRY_REL %2s %d %d %2s %d %d",
            screen_ref, &x_rel, &y_rel, window_ref, &w, &h ) == 6 )
            {
            handled = true;
            if( scalex || scaley )
                {
                x_rel = scalex ? round( x_rel / ratiox ) : x_rel;
                y_rel = scaley ? round( y_rel / ratioy ) : y_rel;
                w = scalex ? round( w / ratiox ) : w;
                h = scaley ? round( h / ratioy ) : h;
                }
            if( !checkRelative( screen_ref )
                || !checkRelative( window_ref ))
                {
                fprintf( stderr,"Bad reference point: %s\n", line );
                exit( 3 );
                }
            x = makeAbsoluteX( screen_ref, x_rel, window_ref, w );
            y = makeAbsoluteY( screen_ref, y_rel, window_ref, h );
            QRect r( x, y, w, h );
            if( screenGeometry(0).contains( r ))
                {
                geometry = r;
                if( window != None )
                    XMoveResizeWindow( qt_xdisplay(), window, x, y, w, h );
                if( !splash_image.isNull())
                    { // destroy and then recreate
                    splash_image = QImage();
                    XFreePixmap( qt_xdisplay(), splash_pixmap );
                    splash_pixmap = None;
                    }
                createSplashImage();
                }
            else
                {
                fprintf( stderr, "Wrong geometry: %s\n", line );
                exit( 3 );
                }
            }
        else if( sscanf( line, "BACKGROUND_IMAGE %d %d %1023s", &x, &y, buf ) == 3 )
            {
            int screens = screenCount();
            handled = true;
            if( scalex || scaley )
                {
                x = scalex ? round( x / ratiox ) : x;
                y = scaley ? round( y / ratioy ) : y;
                }
            if( splash_image.isNull())
                createSplashImage();
            for (int i = 0; i < screens; ++i) {
                QRect geom = screenGeometry(i);
                QImage img = loadImage( buf, geom );
                if( !img.isNull())
                    {
                    blend( img, geom.x() + x, geom.y() + y );
                    updateSplashImage( img, geom.x() + x, geom.y() + y );
                    doPaint( QRect( geom.x() + x, geom.y() + y, img.width(), img.height()));
                    }
                else
                    {
                    fprintf( stderr, "Bad image: %s\n", line );
                    exit( 3 );
                    }
                }
            }
        else if( sscanf( line, "BACKGROUND %1023s", buf ) == 1 )
            {
            handled = true;
            QColor background = QColor( buf );
            QRect geom = totalScreenGeometry();
            if( !background.isValid())
                {
                fprintf( stderr, "Bad color: %s\n", line );
                exit( 3 );
                }
            if( splash_image.isNull())
                createSplashImage();
            splash_image.fill( background.rgb());
            XGCValues xgc;
            xgc.foreground = background.pixel();
            GC gc = XCreateGC( qt_xdisplay(), splash_pixmap, GCForeground, &xgc );
            XFillRectangle( qt_xdisplay(), splash_pixmap, gc, 0, 0, geom.width(), geom.height());
            XFreeGC( qt_xdisplay(), gc );
            doPaint( QRect( 0, 0, geom.width(), geom.height()));
            }
        else if( sscanf( line, "IMAGE %d %d %1023s", &x, &y, buf ) == 3 )
            {
            handled = true;
            if( scalex || scaley )
                {
                x = scalex ? round( x / ratiox ) : x;
                y = scaley ? round( y / ratioy ) : y;
                }
            if( splash_image.isNull())
                createSplashImage();
            QImage img = loadImage( buf, screenGeometry(0) );
            if( !img.isNull())
                {
#if 0
                if( !QRect( 0, 0, geometry.width(), geometry.height())
                    .contains( QRect( x, y, img.width(), img.height())))
                    {
                    fprintf( stderr, "Image outside of geometry: %s\n", line );
                    exit( 3 );
                    }
#endif
                blend( img, x, y );
                updateSplashImage( img, x, y );
                doPaint( QRect( x, y, img.width(), img.height()));
                }
            else
                {
                fprintf( stderr, "Bad image: %s\n", line );
                exit( 3 );
                }
            }
        else if( sscanf( line, "IMAGE_REL %2s %d %d %2s %1023s",
            window_ref, &x_rel, &y_rel, image_ref, buf ) == 5 )
            {
            handled = true;
            if( scalex || scaley )
                {
                x_rel = scalex ? round( x_rel / ratiox ) : x_rel;
                y_rel = scaley ? round( y_rel / ratioy ) : y_rel;
                }
            if( !checkRelative( window_ref )
                || !checkRelative( window_ref ))
                {
                fprintf( stderr,"Bad reference point: %s\n", line );
                exit( 3 );
                }
            if( splash_image.isNull())
                createSplashImage();
            QImage img = loadImage( buf, screenGeometry(0) );
            if( !img.isNull())
                {
                x = makeAbsoluteX( window_ref, x_rel, image_ref, img.width());
                y = makeAbsoluteY( window_ref, y_rel, image_ref, img.height());
#if 0
                if( !QRect( 0, 0, geometry.width(), geometry.height())
                    .contains( QRect( x, y, img.width(), img.height())))
                    {
                    fprintf( stderr, "Image outside of geometry: %s\n", line );
                    exit( 3 );
                    }
#endif
                blend( img, x, y );
                updateSplashImage( img, x, y );
                doPaint( QRect( x, y, img.width(), img.height()));
                }
            else
                {
                fprintf( stderr, "Bad image: %s\n", line );
                exit( 3 );
                }
            }
        items = sscanf( line, "ANIM %d %d %d %d %1023s %d %d", &number, &x, &y, &frames, buf, &delay, &repeat );
        if( items == 6 )
            repeat = 0; // default
        if( items == 6 || items == 7 )
            {
            handled = true;
            if( scalex || scaley )
                {
                x = scalex ? round( x / ratiox ) : x;
                y = scaley ? round( y / ratioy ) : y;
                }
            if( number <= 0 || number >= MAX_ITEMS )
                {
                fprintf( stderr,"Bad number: %s\n", line );
                exit( 3 );
                }
            if( frames <= 0 || frames > MAX_ITEMS )
                {
                fprintf( stderr, "Frames limit reached: %s\n", line );
                exit( 3 );
                }
            if( splash_image.isNull())
                createSplashImage();
            QImage imgs = loadAnimImage( buf, frames );
            if( !imgs.isNull())
                {
                blendAnim( imgs, x, y, frames );
                PixmapData* pixs = imageAnimToPixmaps( imgs, frames );
                delete animations[ number ];
                animations[ number ] = new AnimData( x, y, pixs, frames, delay, repeat );
                }
            }
        items = sscanf( line, "ANIM_REL %d %2s %d %d %2s %d %1023s %d %d",
            &number, window_ref, &x_rel, &y_rel, image_ref, &frames, buf, &delay, &repeat );
        if( items == 8 )
            repeat = 0; // default
        if( items == 8 || items == 9 )
            {
            handled = true;
            if( scalex || scaley )
                {
                x_rel = scalex ? round( x_rel / ratiox ) : x_rel;
                y_rel = scaley ? round( y_rel / ratioy ) : y_rel;
                }
            if( number <= 0 || number >= MAX_ITEMS )
                {
                fprintf( stderr,"Bad number: %s\n", line );
                exit( 3 );
                }
            if( !checkRelative( window_ref )
                || !checkRelative( window_ref ))
                {
                fprintf( stderr,"Bad reference point: %s\n", line );
                exit( 3 );
                }
            if( frames <= 0 || frames > MAX_ITEMS )
                {
                fprintf( stderr, "Frames limit reached: %s\n", line );
                exit( 3 );
                }
            if( splash_image.isNull())
                createSplashImage();
            QImage imgs = loadAnimImage( buf, frames );
            if( !imgs.isNull())
                {
                int framew, frameh;
                frameSize( imgs, frames, framew, frameh );
                x = makeAbsoluteX( window_ref, x_rel, image_ref, framew );
                y = makeAbsoluteY( window_ref, y_rel, image_ref, frameh );
                blendAnim( imgs, x, y, frames );
                PixmapData* pixs = imageAnimToPixmaps( imgs, frames );
                delete animations[ number ];
                animations[ number ] = new AnimData( x, y, pixs, frames, delay, repeat );
                }
            }
        else if( sscanf( line, "STOP_ANIM %d", &number ) == 1 )
            {
            handled = true;
            if( number <= 0 || number >= MAX_ITEMS || animations[ number ] == NULL )
                {
                fprintf( stderr,"Bad number: %s\n", line );
                exit( 3 );
                }
            AnimData* anim = animations[ number ];
            doPaint( QRect( anim->x, anim->y, anim->frames[ 0 ].w, anim->frames[ 0 ].h ));
            delete animations[ number ];
            animations[ number ] = NULL;
            }
        else if( sscanf( line, "WAIT_STATE %s", buf ) == 1 )
            {
            handled = true;
            int new_state = -1;
            for( int i = 0;
                 i < int( sizeof( states ) / sizeof( states[ 0 ] ));
                 ++i )
                {
                if( strcmp( buf, states[ i ] ) == 0 )
                    {
                    new_state = i;
                    break;
                    }
                }
            if( new_state == -1 )
                {
                fprintf( stderr, "Unknown splash state: %s\n", buf );
                // don't make fatal, may be a theme for a newer version
                }
            else
                {
                if( waitState( new_state ))
                    break; // exiting
                }
            }
        if( !handled )
            {
            fprintf( stderr, "Unknown line: %s\n", line );
            exit( 3 );
            }
        }
    fclose( datafile );
    XDestroyWindow( qt_xdisplay(), window );
    window = None;
    XFreePixmap( qt_xdisplay(), splash_pixmap );
    splash_pixmap = None;
    splash_image = QImage();
    }
