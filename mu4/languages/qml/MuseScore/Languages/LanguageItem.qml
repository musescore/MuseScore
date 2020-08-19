import QtQuick 2.9

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Languages 1.0

Rectangle {
    id: root

    property string title: ""
    property string statusTitle: ""

    property real headerWidth: width / 2
    property real sideMargin: 0.0

    signal clicked()

    height: 48

    color: ui.theme.backgroundPrimaryColor

    Row {
        anchors.left: parent.left
        anchors.leftMargin: sideMargin
        anchors.right: parent.right
        anchors.rightMargin: sideMargin

        anchors.verticalCenter: parent.verticalCenter

        Row {
            width: headerWidth

            spacing: 12

            StyledIconLabel {
                iconCode: IconCode.NEW_FILE
            }
            StyledTextLabel {
                text: title
                font.pixelSize: 14
                horizontalAlignment: Text.AlignLeft
            }
        }

        StyledTextLabel {
            width: headerWidth

            text: statusTitle
            font.pixelSize: 14
            horizontalAlignment: Text.AlignLeft
        }
    }

    MouseArea {
        anchors.fill: parent

        onClicked: {
            root.clicked()
        }
    }
}
