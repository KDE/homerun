/*
 *   Copyright 2012 Aurélien Gâteau <agateau@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
// KDE
#include <KAboutData>
#include <KCmdLineArgs>
#include <KDebug>
#include <KLocale>
#include <KUniqueApplication>
#include <kdeclarative.h>

// Qt
#include <QScopedPointer>

// Local
#include <aboutdata.h>
#include <fullview.h>
#include <homerun_config.h>

int main(int argc, char *argv[])
{
    QScopedPointer<KAboutData> aboutData(HomerunInternal::createAboutData());

    // Define cmdline options
    KCmdLineArgs::init(argc, argv, aboutData.data());
    KCmdLineOptions options;
    options.add("show <screen>", ki18n("Show on screen <screen>"), "-1");
    options.add("log-focused-item", ki18n("Log focused item (for debug purposes)"));
    options.add("plain-window", ki18n("Use a plain window (for debug purposes)"));
    KCmdLineArgs::addCmdLineOptions(options);
    KUniqueApplication::addCmdLineOptions();

    // Create app
    if (!KUniqueApplication::start()) {
        kError() << "Already running.";
        return 0;
    }

    KUniqueApplication app;
    app.disableSessionManagement();
#ifdef HAVE_KDECLARATIVE_SETUPQMLJSDEBUGGER
    KDeclarative::setupQmlJsDebugger();
#endif

    // Create view
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    KCmdLineArgs *kdeArgs = KCmdLineArgs::parsedArgs("kde");

    if (!args->isSet("plain-window")) {
        app.setQuitOnLastWindowClosed(false);
    }

    FullView view;

    view.setConfigFileName(
        kdeArgs->isSet("config")
        ? kdeArgs->getOption("config")
        : "homerunrc");

    if (args->isSet("show")) {
        QString str = args->getOption("show");
        bool ok;
        int screen = str.toInt(&ok);
        if (!ok) {
            kError() << "Invalid screen number:" << str << ". Using screen 0.";
            screen = 0;
        }
        view.toggle(screen);
    }
    return app.exec();
}
