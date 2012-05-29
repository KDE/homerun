# Features of a SAL-friendly model

## Roles
### string label
- mandatory: yes
Item text

### string|QIcon icon
- mandatory: yes
Name of the item icon

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
Triggers the action associated with the item at row "row". Returns true if SAL
should close itself.

### string favoriteAction(ModelData modelData)
- mandatory: yes
Indicates what "favorite" action the item represented by "modelData" supports.
Can return either:
- "": no action possible
- "add": item can be "favorited"
- "remove": item can be "un-favorited"

### void triggerFavoriteAction(ModelData modelData)
- mandatory: yes
Triggers the "favorite" action described by the favoriteAction() method.