# X Add a config menu-button to main screen

Content:
- Configure

Position: top-right, next to the search field

# X Introduce "Configure" mode in main.qml

# X When entering configure mode, delete all TabContent and create a new Item

TabContentEditor.qml

SourceItem.qml

# X Add a "Remove" button to SourceItem

# X Propagate removal to config file

# X Show available sources in SourceItem

# X Add a new SourceItem to TabContentEditor when selected

# X Propagate new source to config file

# X Add up|down buttons (or drag'n'drop) to SourceItem

# X Propagate source reordering to config file

# X Add line edit to SourceItem for primitive configuration

# X Add a way for sources to provide a configuration widget

- X Add "createConfigurationWidget()" method to AbstractSource
- X Use it in SourceItem
- X To test, implement it in DirModel

# X Show source "visible name" in configuration

# X Recreate selectedSourcesModel in Page.qml

# X Expose again necessary properties to SourceItem

# X Move availableSourcesModel to Page.qml

# X Show list of available sources in Page.qml

# _ Implement createConfigurator() in other sources
