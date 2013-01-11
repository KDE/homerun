# Forewords

This document tries to provide a high-level overview of Homerun code. Those kind
of documents are always in danger of becoming obsolete. If you find that
something written there is unclear or looks outdated, get in touch with us so
we can improve or refresh it.

# Developing sources

If you want to create your own sources, make sure you build libhomerun
documentation with `make dox` and have a look at the `helloworld` dir in this dir.

# QML side

Main window is in main.qml

main.qml contains a set of tabs, the content of each tab is handled by
TabContent.

The TabContent item handles a stack of pages. Most tabs have only one page but
some sources, like the Dir source or the InstalledApps source, allow
navigation. In this case each new level of navigation is created as a new page.

The Page item handles the content of a page. It mainly contains a ListView
whose items are made of a Column containing SourceItem and source views.

SourceItem is the thin box which appears in configuration mode and allows
editing, reordering and removing a source.

The model of a source usually directly provides the data to display, in which
case the ListView delegate contains only one ResultsView. Source model may
also provide sub-models, in which case the delegate will a Column which contains
one ResultsView per sub-model.

A ResultsView is made of a grid view which contains one or more Result items.
Each Result displays one element of the ResultsView model.

To sum up, the architecture looks like this:

- main.qml
  - TabContent
    - Page
      - Column
        - SourceItem
        - ResultsView
          - Result
          - ...
          - ...
          - ...
      - Column
        - SourceItem (The source associated with this item provides 2 sub-models)
        - Column
          - ResultsView
            - Result
            - ...
            - ...
          - ResultsView
            - Result
            - ...
      - Column
        - SourceItem
        - ResultsView
          - Result
        - ...
    - Page (Only if first page contained a browsable source)
      -Column
        - SourceItem
        - ResultsView
          - Result
          - ...
  - TabContent
  - TabContent
  - TabContent
