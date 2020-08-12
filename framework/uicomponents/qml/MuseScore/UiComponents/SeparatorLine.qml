import QtQuick 2.0
import MuseScore.Ui 1.0

Rectangle {
    id: root

    property int orientation: Qt.Horizontal

    color: ui.theme.strokeColor

    states: [
        State {
            name: "HORIZONTAL"
            when: orientation == Qt.Horizontal

            PropertyChanges {
                target: anchors
                left: parent.left
                right: parent.right
            }

            PropertyChanges {
                target: root
                height: 1
            }
        },

        State {
            name: "VERTICAL"
            when: orientation == Qt.Vertical

            PropertyChanges {
                target: anchors
                top: parent.top
                bottom: parent.bottom
            }

            PropertyChanges {
                target: root
                width: 1
            }
        }
    ]
}
