# Package Manager integration

You can integrate your package manager with sal so that sal can start your
package manager to install|review|remove software.

## Installation

When browsing installed applications by category, sal can provide an icon to
install more applications from this category. To do so you need to:

1. Provide a .desktop file whose Exec= line contains a "@category" placeholder.
   This placeholder will be replaced by the category sal is browsing.
   For example, here is muon-discover-category.desktop:

    [Desktop Entry]
    Name=Install More...
    Exec=muon-discover --category @category
    Icon=muondiscover
    Type=Application
    NoDisplay=true

2. In "salrc", "PackageManagement" group, set the "categoryInstaller" to the
   name of your .desktop file (without the .desktop extension)
