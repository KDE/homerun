# X Add a config menu-button to main screen

Rationale:
- It is simpler to implement in main screen than in the launcher configuration
  dialog because it is the same process.

- In the long run configuration should be done inline, not through a dialog,
  which means not using the launcher configuration dialog.

Position: top-right, next to the search field

# X Show config dialog from the menu

# _ List tabs in dialog

# _ List available sources

# _ List active sources

# _ Make it possible to activate/deactivate sources

# _ Make it possible to reorder sources

# _ Add line-edit for primitive source configuration

# _ Add a way for sources to provide a configuration widget

- _ Add "createConfigurator()" method to AbstractSource
- _ Use it in config dialog
- _ Implement it in DirModel

# _ Implement createConfigurator() in other sources
