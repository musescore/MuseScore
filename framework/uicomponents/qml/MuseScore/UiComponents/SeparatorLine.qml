import QtQuick 2.9
import QtQuick.Layouts 1.3

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
                target: root
                height: 1
                Layout.fillWidth: true
            }
        },

        State {
            name: "VERTICAL"
            when: orientation == Qt.Vertical

            PropertyChanges {
                target: root
                width: 1
                Layout.fillHeight: true
            }
        }
    ]
}
