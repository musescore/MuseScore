import QtQuick 2.9
import QtQuick.Controls 2.2

import MuseScore.Ui 1.0

ScrollBar {
    id: root

    property alias color: handle.color
    property alias border: handle.border

    width: orientation === Qt.Vertical ? 10 : 0
    height: orientation === Qt.Horizontal ? 10 : 0

    visible: size !== 0 && size !== 1
    padding: 0

    contentItem: Rectangle {
        id: handle

        radius: 5
        color: ui.theme.fontPrimaryColor
        opacity: root.pressed ? 0.7 : 0.3
        visible: root.active
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
