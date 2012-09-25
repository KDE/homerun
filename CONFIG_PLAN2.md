# X Add a config menu-button to main screen

Content:
- Configure

Position: top-right, next to the search field

# X Introduce "Configure" mode in main.qml

# _ In TabContent.qml, show a different Item for each ResultsView in Configure Mode

Name of item: ConfigureSource.qml

# _ Add a "Remove" button to ConfigureSource.qml

# _ Propagate removal to config file

# _ Add a "Add Source" button at the end of TabContent.qml

# _ Show available sources in a Plasma dialog when "Add Source" is clicked

# _ Add the new source to TabContent when selected

# _ Propagate new source to config file

# _ Add up|down buttons (or drag'n'drop) to ConfigureSource

# _ Propagate source reordering to config file

# _ Add line edit to ConfigureSource for primitive configuration

# _ Add a way for sources to provide a configuration QML file

- _ Add "createConfigurator()" method to AbstractSource
- _ Use it in ConfigureSource
- _ To test, implement it in DirModel

# _ Implement createConfigurator() in other sources
