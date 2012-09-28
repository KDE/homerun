# X Add a config menu-button to main screen

Content:
- Configure

Position: top-right, next to the search field

# X Introduce "Configure" mode in main.qml

# X When entering configure mode, delete all TabContent and create a new Item

TabContentEditor.qml

SourceEditor.qml

# X Add a "Remove" button to SourceEditor

# X Propagate removal to config file

# X Show available sources in SourceEditor

# X Add a new SourceEditor to TabContentEditor when selected

# X Propagate new source to config file

# X Add up|down buttons (or drag'n'drop) to SourceEditor

# X Propagate source reordering to config file

# X Add line edit to SourceEditor for primitive configuration

# X Add a way for sources to provide a configuration widget

- X Add "createConfigurationWidget()" method to AbstractSource
- X Use it in SourceEditor
- X To test, implement it in DirModel

# _ Implement createConfigurator() in other sources

# X Show source "visible name" in configuration
