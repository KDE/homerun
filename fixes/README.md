# Intro

This dir contains patched copies of classes provided by other projects.

Those copies are intended to be temporary and should be removed when fixes are
upstreamed and available on all machines used to test SAL. If a fix cannot be
upstreamed and there is no other workaround, the class should be forked and
moved to the components/ dir.

# Current Status

## SortFilterModel
### Fix for flickering
https://git.reviewboard.kde.org/r/105042/

### New methods: mapRowFromSource, mapRowToSource
https://git.reviewboard.kde.org/r/105044/

## RunnerModel
### Do not reset Plasma::RunnerModel when new matches are added
https://git.reviewboard.kde.org/r/104917/

### Fix behavior of "runners" property
https://git.reviewboard.kde.org/r/105024/
