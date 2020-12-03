import QtQuick 2.5

FocusScope {
    id: root

    activeFocusOnTab: true

    MouseArea {
        anchors.fill: parent

        preventStealing: true

        onClicked: {
            if (!activeFocus) {
                root.forceActiveFocus()
            }
        }
    }
}
