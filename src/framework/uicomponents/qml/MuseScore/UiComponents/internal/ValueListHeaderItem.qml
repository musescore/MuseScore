import QtQuick 2.15

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Item {
    id: root

    property string headerTitle: ""
    property alias spacing: row.spacing
    property bool isSorterEnabled: false
    property int sortOrder: Qt.AscendingOrder

    signal clicked()

    Row {
        id: row

        anchors.fill: parent
        spacing: root.spacing

        StyledTextLabel {
            id: titleLabel
            anchors.verticalCenter: parent.verticalCenter
            width: implicitWidth

            text: headerTitle
            horizontalAlignment: Text.AlignLeft
            font.capitalization: Font.AllUppercase
            opacity: ui.theme.buttonOpacityNormal
        }

        StyledIconLabel {
            id: sorterIcon
            anchors.verticalCenter: parent.verticalCenter

            visible: isSorterEnabled
            iconCode: sortOrder === Qt.AscendingOrder ? IconCode.SMALL_ARROW_DOWN : IconCode.SMALL_ARROW_UP

            opacity: ui.theme.buttonOpacityNormal
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        hoverEnabled: true

        onClicked: {
            root.clicked()
        }
    }

    states: [
        State {
            name: "HOVERED"
            when: mouseArea.containsMouse && !mouseArea.pressed

            PropertyChanges {
                target: titleLabel
                opacity: ui.theme.buttonOpacityHover
            }

            PropertyChanges {
                target: sorterIcon
                opacity: ui.theme.buttonOpacityHover
            }
        },

        State {
            name: "SELECTED"
            when: root.isSorterEnabled

            PropertyChanges {
                target: titleLabel
                opacity: ui.theme.buttonOpacityHit
            }

            PropertyChanges {
                target: sorterIcon
                opacity: ui.theme.buttonOpacityHit
            }
        }
    ]
}
