import QtQuick 2.9
import QtQuick.Controls 2.2

import MuseScore.Ui 1.0

ScrollBar {
    id: root

    width: orientation === Qt.Vertical ? 10 : 0
    height: orientation === Qt.Horizontal ? 10 : 0

    visible: size !== 0

    property var color: ui.theme.fontPrimaryColor

    property var withBorder: false
    property var borderColor: ui.theme.fontPrimaryColor

    contentItem: Rectangle {
        radius: 5
        color: root.color
        opacity: root.pressed ? 0.7 : 0.3
        visible: root.active
        border.width: root.withBorder ? 1 : 0
        border.color: root.borderColor
    }

    function setPosition(position) {
        root.position = position

        root.active = true

        if (!resetActiveTimer.running) {
            resetActiveTimer.stop()
        }

        resetActiveTimer.start()
    }

    Timer {
        id: resetActiveTimer

        onTriggered: {
            root.active = Qt.binding( function() { return root.hovered || root.pressed } )
        }
    }
}
