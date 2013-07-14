/*
Copyright 2012 Aurélien Gâteau <agateau@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
// KDE
#include <KAboutData>
#include <KBugReport>
#include <KCmdLineArgs>
#include <KDebug>
#include <KLocale>
#include <KMessageBox>
#include <KUniqueApplication>
#include <kdeclarative.h>

// Qt
#include <QScopedPointer>

// Local
#include <aboutdata.h>
#include <fullview.h>
#include <homerun_config.h>

static void showError(const QString &errorMessage)
{
    int ret = KMessageBox::warningContinueCancel(0,
        i18n("<p><b>Sorry, Homerun failed to load</b></p>"
            "<p>Do you want to report this error?</p>"
            "<p>If you decide to report it, please copy and paste the following output in your report:</p>"
            "<pre>%1</pre>",
            errorMessage
            ),
        QString(), // caption
        KGuiItem(i18n("Report Error")),
        KStandardGuiItem::close()
        );
    if (ret != KMessageBox::Continue) {
        return;
    }
    KBugReport report;
    report.exec();
}

int main(int argc, char *argv[])
{
    QScopedPointer<KAboutData> aboutData(HomerunInternal::createAboutData());

    // Define cmdline options
    KCmdLineArgs::init(argc, argv, aboutData.data());
    KCmdLineOptions options;
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
    KDeclarative::setupQmlJsDebugger();

    // Create view
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    KCmdLineArgs *kdeArgs = KCmdLineArgs::parsedArgs("kde");

    if (!args->isSet("plain-window")) {
        app.setQuitOnLastWindowClosed(false);
    }

    FullView view;

    QString errorMessage;
    if (!view.init(&errorMessage)) {
        if (args->isSet("show")) {
            showError(errorMessage);
        }
        return 1;
    }

    view.setConfigFileName(
        kdeArgs->isSet("config")
        ? kdeArgs->getOption("config")
        : "homerunrc");

    return app.exec();
}
