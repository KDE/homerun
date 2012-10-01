# Forewords

This document tries to provide a high-level overview of Homerun code. Those kind
of documents are always in danger of becoming obsolete. If you find that
something written there is unclear or looks outdated, get in touch with us so
we can improve or refresh it.

# QML side

Main window is in main.qml

main.qml contains a set of tabs, the content of each tab is handled by
TabContent.

The TabContent item handles a stack of pages. Most tabs have only one page but
some sources, like the Dir source or the InstalledApps source, allow
navigation. In this case each new level of navigation is created as a new page.

The Page item handles the content of a page. It mainly contains a ListView
whose items are SourceEditor, there is one SourceEditor per Source.

A SourceEditor is a thin decoration around one or more ResultsView. Unless user
is configuring Homerun, SourceEditor does not have any UI, it just shows the
ResultsViews of its source.

The model of a source usually directly provide the data to display, in w
hich case the SourceEditor contains only one ResultsView. Source model may a
lso provide sub-models, in which case the SourceEditor will contain one
ResultsView per sub-model.
