#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.rc -o -name \*.ui -o -name \*.kcfg` >> rc.cpp
$XGETTEXT `find . -name \*.cpp -o -name \*.qml` -o $podir/plasma_applet_org.kde.homerun.pot
rm -f rc.cpp
