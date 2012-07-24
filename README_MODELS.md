# Features of a SAL-friendly model

## Roles
### string label
- mandatory: yes
Item text

### string|QIcon icon
- mandatory: yes
Name of the item icon

### string favoriteId
- mandatory: no
Unique Id used to favorite this item. Its value depends on the item type:

- item cannot be favorited: ""
- item is an application: "app:" + $desktopName.
  Examples: "app:kde4-dolphin.desktop" or "app:inkscape.desktop".
  for KDE applications)
- item is a place: "place:" + $url.
  Examples: "place:file:///home/jdoe/Documents" or "place:sftp://host.com/dir".

## Properties
### string name
- access: read-only
- mandatory: yes
Returns a translated name for the model. This name is used as a title for the
view associated with the model.

### int count
- access: read-only
- mandatory: yes
Returns current number of rows. Most often it will simply return
rowCount(QModelIndex()).

### string path
- access: read-write
- mandatory: no
Used for models which can be navigated through. Should start with "/".

### array(string) arguments
- access: read-write
- mandatory: no
Define optional model arguments.

## Methods
### bool trigger(int row)
- mandatory: yes
Triggers the action associated with the item at row "row".
Returns true if the trigger started an action outside of SAL, false if it did
not. Returning false can happen if the action failed or if it was handled in
another way, for example because the model emitted openSourceRequested().

## Signals
## openSourceRequested(QString source)
- mandatory: no
Emitted by a model when it wants another source to be opened by the view. This
is used for example to open a browsable model in a subdir.



# Features of a SAL-friendly favorite model

## Properties
### string favoritePrefix
- access: read-only
- mandatory: yes

Returns the favoriteId prefix this model handles. For example, the model storing
favorite applications returns "app". The model storing favorite places returns
"place".

## Methods
### bool isFavorite(string favoriteId) const
- mandatory: yes

### void addFavorite(string favoriteId)
- mandatory: yes

### void removeFavorite(string favoriteId)
- mandatory: yes
