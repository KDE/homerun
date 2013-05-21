# Homerun

## What is it?

Homerun is a KDE application launcher and document opener.

It is a fullscreen launcher with content organized in tabs. A tab is made of
several "sources". A source can provide one or more sections to a tab. Homerun
comes with a few built-in sources, but custom sources can be written using
libhomerun.

Homerun allows you to start applications, but also to browse hierarchies: for
example clicking on an item of the "Favorite Places" section won't start
Dolphin in this folder: it will list the folder content in Homerun itself,
making it possible to explore the filesystem and directly open the file you are
looking for. In this aspect it can be seen as an alternative to the Folder View
applet.

Homerun can either be used from a launcher button in a panel, like Kickoff, or
as a containment, like the Search and Launch containment.

## Dependencies

kdelibs 4.10

## Build instructions

Homerun currently requires kdelibs and kde-runtime 4.10.

To build it, do the following:

    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=`kde4-config --prefix` ..

Assuming cmake did not complain about missing dependencies, run:

    make

Then to install it, run as root:

    make install

## For packagers: Package manager integration

You can integrate your package manager with Homerun.

When browsing installed applications by category, Homerun can provide an item to
install more applications from this category. To do so you need to:

First, create a .desktop file whose Exec= line contains a "@category" placeholder.
This placeholder will be replaced by the category Homerun is browsing.

For example, here is muon-discover-category.desktop:

    [Desktop Entry]
    Name=Install More...
    Exec=muon-discover --category @category
    Icon=muondiscover
    Type=Application
    NoDisplay=true

Then in "homerunrc", "PackageManagement" group, set the value of the
"categoryInstaller" entry to the name of your .desktop file (without the
.desktop extension)

## Creating your own sources

It is possible to create your own sources using libhomerun.

To learn more about libhomerun, run `make dox` to generate the API
documentation. The generated documentation will be in "$BUILDDIR/lib/html".

You should also have a look at the code of the "helloworld" example source in
"devdoc/helloworld".

## Getting involved

You can get in touch with us on IRC: #kde-homerun on Freenode.

You can report bugs on <https://bugs.kde.org>, product "homerun".

Clone the git repository with:

    git clone git://anongit.kde.org/homerun

File review requests on <http://git.reviewboard.kde.org>, repository "homerun".
