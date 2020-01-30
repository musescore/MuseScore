import QtQuick 2.5

FocusScope {
    id: root

    activeFocusOnTab: true

    MouseArea {
        anchors.fill: parent

        hoverEnabled: true

        onContainsMouseChanged: {
            if (containsMouse) {
                root.forceActiveFocus()
            }
        }
    }
}
