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

            AnchorChanges {
                target: root
                anchors.left: parent.left
                anchors.right: parent.right
            }

            PropertyChanges {
                target: root
                height: 1
            }
        },

        State {
            name: "VERTICAL"
            when: orientation == Qt.Vertical

            AnchorChanges {
                target: root
                anchors.top: parent.top
                anchors.bottom: parent.bottom
            }

            PropertyChanges {
                target: root
                width: 1
            }
        }
    ]
}
