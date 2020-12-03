import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

Item {
    id: root

    function load(templatePath) {
        paintView.load(templatePath)
    }

    function zoomIn() {
        paintView.zoomIn()
    }

    function zoomOut() {
        paintView.zoomOut()
    }

    StyledTextLabel {
        id: title

        anchors.top: parent.top

        text: qsTrc("userscores", "Preview")
        font.bold: true
    }

    Item {
        anchors.top: title.bottom
        anchors.topMargin: 16
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        TemplatePaintView {
            id: paintView

            anchors.fill: parent
        }

        ScrollBar {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            z: 1
            orientation: Qt.Vertical
            policy: ScrollBar.AlwaysOn

            position: paintView.startVerticalScrollPosition
            size: paintView.verticalScrollSize

            onPositionChanged: {
                if (pressed) {
                    paintView.scrollVertical(position)
                }
            }
        }

        ScrollBar {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            z: 1
            orientation: Qt.Horizontal
            policy: ScrollBar.AlwaysOn

            position: paintView.startHorizontalScrollPosition
            size: paintView.horizontalScrollSize

            onPositionChanged: {
                if (pressed) {
                    paintView.scrollHorizontal(position)
                }
            }
        }
    }
}
