import QtQuick 2.0

import MuseScore.UiComponents 1.0

FocusableItem {
    id: root

    property QtObject model: undefined

    property var contentHeight: implicitHeight
    signal contentExtended()

    anchors.left: parent.left
    anchors.right: parent.right
    anchors.rightMargin: 48

    onContentHeightChanged: {
        if (contentHeight > implicitHeight) {
            root.contentExtended()
        } else if (contentHeight < implicitHeight) {
            root.contentIntended()
        }
    }
}
