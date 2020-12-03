import QtQuick 2.9
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0

Rectangle {
    id: root

    property int orientation: Qt.Horizontal

    color: ui.theme.strokeColor

    QtObject {
        id: privateProperties
        function parentIsLayout() {
            return root.parent instanceof ColumnLayout || root.parent instanceof RowLayout
        }
    }

    states: [
        State {
            name: "HORIZONTAL"
            when: orientation == Qt.Horizontal

            PropertyChanges {
                target: root
                height: 1
                Layout.fillWidth: true
            }

            StateChangeScript {
                script: {
                    if (privateProperties.parentIsLayout()) {
                        root.Layout.fillWidth = true
                    } else {
                        root.anchors.left = root.parent.left
                        root.anchors.right = root.parent.right
                    }
                }
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

            StateChangeScript {
                script: {
                    if (privateProperties.parentIsLayout()) {
                        root.Layout.fillHeight = true
                    } else {
                        root.anchors.top = root.parent.top
                        root.anchors.bottom = root.parent.bottom
                    }
                }
            }
        }
    ]
}
