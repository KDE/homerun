# Intro

This dir contains patched or backported copies of classes provided by other
projects.

Those copies are intended to be temporary and should be removed when fixes are
upstreamed and available on all machines used to test Homerun. If a fix cannot
be upstreamed and there is no other workaround, the class should be forked and
moved to the components/ dir.

# Current Status

## TextField.qml (kde-runtime)

### Keep placeholder text visible when focused and empty
<https://git.reviewboard.kde.org/r/107678/>

Applied in 4.11

## SortFilterModel (kde-runtime)

### Fix crash in mapRowFromSource()
<https://git.reviewboard.kde.org/r/109946/>

Applied in 4.11

### Emit beginMoveRows() and endMoveRows()

Need to be proposed

## KFilePlacesModel (kdelibs)

### Emit rowsMoved() when reordering via drag and drop, instead of remove+insert
<https://git.reviewboard.kde.org/r/109935/>

Applied in 4.11

Files for the fixed KFilePlacesModel class are in `components/sources/favorites/`.
