# X Add a config menu-button to main screen

Content:
- Configure

Position: top-right, next to the search field

# X Introduce "Configure" mode in main.qml

# _ When entering configure mode, delete all TabContent and create a new Item

TabContentEditor.qml

SourceEditor.qml

# _ Add a "Remove" button to SourceEditor

# _ Propagate removal to config file

# _ Add a "Add Source" button at the end of TabContentEditor

# _ Show available sources in a Plasma dialog when "Add Source" is clicked

# _ Add a new SourceEditor to TabContentEditor when selected

# _ Propagate new source to config file

# _ Add up|down buttons (or drag'n'drop) to SourceEditor

# _ Propagate source reordering to config file

# _ Add line edit to SourceEditor for primitive configuration

# _ Add a way for sources to provide a configuration QML file

- _ Add "createConfigurator()" method to AbstractSource
- _ Use it in SourceEditor
- _ To test, implement it in DirModel

# _ Implement createConfigurator() in other sources
