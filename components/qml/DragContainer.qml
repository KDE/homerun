import QtQuick 1.1

Item {
    property QtObject model

    property int draggedIndex: -1

    function indexAt(viewX, viewY) {
        return parent.indexAt(parent.contentX + viewX, parent.contentY + viewY);
    }
}
