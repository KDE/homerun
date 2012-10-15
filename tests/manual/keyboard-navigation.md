# startup

- Focus is on first item of first source
- Typing letters gets added to the search field

# tabs

- Switching to another tab should focus first item of first source

# sourceNavigation

- Using arrow keys within source changes the focused item accordingly

# interSourceNavigation

- Pressing up/down arrow when on first/last row of source moves focus
  to previous/next source, preferably to item in the same column,
  otherwise to last item in column

- Pressing left/right arrow on first/last row of source moves focus to
  last/first item of previous/next source

# browsableSource

- Go to a browsable source (Dir, FavoritePlaces, InstalledApps...)
- Press Return to enter a folder => first item of folder content should
  be focused
- Press alt-left to go up one level => previously focused item is focused

# filtering

- ensure an item is focused
- enter a search criteria which does not match focused item
  => item next to it should be focused
- enter a search criteria which does not match any item of the source
  => first matching item of next source should be focused. If there is none,
  then last matching item of previous source should be focused.
