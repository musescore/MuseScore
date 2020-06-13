import QtQuick 2.5

FocusScope {
    id: root

    activeFocusOnTab: true

    MouseArea {
        anchors.fill: parent

        propagateComposedEvents: true

        // ensure that mouse area is on the top of z order
        // mouse event will be propagated to other overlapped mouse areas
        z: 1000

        onClicked: {
            if (!activeFocus) {
                root.forceActiveFocus()
            }

            mouse.accepted = false
        }

        onPressed: {
            mouse.accepted = false
        }

        onReleased: {
            mouse.accepted = false
        }
    }
}
