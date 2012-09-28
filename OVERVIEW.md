# Forewords

This document tries to provide a high-level overview of Homerun code. Those kind
of documents are always in danger of becoming obsolete. If you find that
something written there is unclear or looks outdated, get in touch with us so
we can improve or refresh it.

# QML side

Main window is in main.qml

main.qml contains a set of tabs, the content of each tab is handled by
TabContent.

The TabContent item handles a stack of pages. Most tabs have only one page, but
some sources, like the Dir source or the InstalledApps source, allows
navigation. In this case each new level of navigation is created as a new page.

The Page item handles the content of a page. It is made of a list of ResultsView
items.
