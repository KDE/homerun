# Intro

This dir contains patched or backported copies of classes provided by other
projects.

Those copies are intended to be temporary and should be removed when fixes are
upstreamed and available on all machines used to test Homerun. If a fix cannot
be upstreamed and there is no other workaround, the class should be forked and
moved to the components/ dir.

# Current Status

## Label.qml (kde-runtime)
A copy of the 4.9 version. The 4.8 version imports QtQuick 1.0, but we need
QtQuick 1.1 to be able to use properties like "truncated" or "maximumLineCount".

## TextField.qml (kde-runtime)
A copy of the 4.9 version. The 4.8 version has a bug with dark themes like
Oxygen: it use white text on white background.

### Keep placeholder text visible when focused and empty
<https://git.reviewboard.kde.org/r/107678/>

Not applied for now

## ScrollArea (kde-runtime)
A copy from kde-runtime master. This is needed because this component does not
exist in KDE 4.8.
