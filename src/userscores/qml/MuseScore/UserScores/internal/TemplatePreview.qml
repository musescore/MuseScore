import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

Item {
    id: root

    function load(templatePath) {
        templateView.load(templatePath)
    }

    function zoomIn() {
        templateView.zoomIn()
    }

    function zoomOut() {
        templateView.zoomOut()
    }

    StyledTextLabel {
        id: title

        anchors.top: parent.top

        text: qsTrc("userscores", "Preview")
        font: ui.theme.bodyBoldFont
    }

    TemplatePaintView {
        id: templateView

        anchors.top: title.bottom
        anchors.topMargin: 16
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        onHorizontalScrollChanged: {
            if (!horizontalScrollBar.pressed) {
                horizontalScrollBar.setPosition(templateView.startHorizontalScrollPosition)
            }
        }

        onVerticalScrollChanged: {
            if (!verticalScrollBar.pressed) {
                verticalScrollBar.setPosition(templateView.startVerticalScrollPosition)
            }
        }

        StyledScrollBar {
            id: verticalScrollBar

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            orientation: Qt.Vertical

            position: templateView.startVerticalScrollPosition
            size: templateView.verticalScrollSize

            onPositionChanged: {
                if (pressed) {
                    templateView.scrollVertical(position)
                }
            }
        }

        StyledScrollBar {
            id: horizontalScrollBar

            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            orientation: Qt.Horizontal

            position: templateView.startHorizontalScrollPosition
            size: templateView.horizontalScrollSize

            onPositionChanged: {
                if (pressed) {
                    templateView.scrollHorizontal(position)
                }
            }
        }
    }
}
