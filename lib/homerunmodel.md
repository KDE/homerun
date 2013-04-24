Homerun Model Specification {#homerunmodel}
===========================

Homerun models make heavy use of Qt introspection and duck-typing. This
document describes the mandatory and optional roles, properties and signals of a
Homerun model.

## Mandatory elements

### Roles
#### string display

The item text.

#### string|QIcon decoration

The item icon, as a string or as a QIcon.

### Properties
#### string name
- access: read-only

Returns a translated name for the model. This name is used as a title for the
view associated with the model.

#### int count
- access: read-only

Returns current number of rows. Most often it will simply return
rowCount(QModelIndex()).

### Methods
#### bool trigger(int row, string actionId = "", QVariant actionArgument = QVariant())

Triggers the action identified by `actionId` on the item at row `row`.
`actionId` and `actionArgument` are defined in the `actionList` role.

Returns true if you want Homerun to close, false otherwise.

Note: When the user clicks on an item, `actionId` is an empty string.




## To create items which can be favorited

### Roles
#### string favoriteId

Unique Id used to favorite this item. Its value depends on the item type:

- item cannot be favorited: ""
- item is an application: "app:" + $desktopName.
  Examples: "app:inkscape.desktop" or "app:kde4-dolphin.desktop" for KDE applications.
- item is a place: "place:" + $url.
  Examples: "place:file:///home/jdoe/Documents" or "place:sftp://host.com/dir".




## To create items with actions

### Roles
#### bool hasActionList

Set to true if the item has additional actions, defined by the actionList
role.

#### QVariantList actionList

A list of QVariantMap describing extra actions. Each map should contain the
following elements:

- string type
    - mandatory: no, default to "action"
    - Can be one of "action", "title" or "separator"
- string text
    - mandatory: yes
    - The visible text shown in the item context menu
- QIcon icon
    - mandatory: no
    - The icon associated with the action
- string actionId
    - mandatory: yes
    - An id passed to trigger() to distinguish between actions
- variant actionArgument
    - mandatory: no
    - An optional argument, passed to trigger() as well
- bool enabled
    - mandatory: no
    - If set to true or omitted, the item will be enabled. If set to false, it will be disabled

Functions from the Homerun::ActionList namespace simplify the creation of
actions.

Important: This role is ignored if hasActionList is not defined or returns
false.




## To create browsable models

### Properties
#### Homerun::PathModel pathModel
- access: read-only

This property must return a Homerun::PathModel instance representing the
position inside the model.

### Signals
#### openSourceRequested(QString sourceId, const QVariantMap &arguments)

Emitted by a model when it wants another source to be opened by the view. This
is used for example to open a browsable model in a subdir.

Note: If you want to be able to access your model this way, you need to
implement Homerun::AbstractSource::createModelFromArguments().




## To give running feedback

### Properties
#### bool running
- access: read-only

Set to true to indicate the model is busy filling itself. Homerun will show a
busy indicator as long as this property is true.




## To provide custom searching/filtering

### Properties
#### string query
- access: read-write

A model can implement this property to implement searching/filtering itself. If
it is not defined, Homerun will apply a generic filter to the item names when
the user type a search criteria.




## To let the user reorder items

### Properties
#### bool canMoveRow
- access: read-only

Implement this property and make it return true.

### Methods
#### void moveRow(int from, int to)

This method should move row `from` to row `to` in the model and take care of
any necessary serialization.
