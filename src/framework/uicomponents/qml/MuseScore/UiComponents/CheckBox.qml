import QtQuick 2.0
import QtQuick.Layouts 1.3
import MuseScore.Ui 1.0

FocusableItem {
    id: root

    property bool checked: false
    property bool isIndeterminate: false

    property alias text: label.text
    property alias font: label.font
    property alias wrapMode: label.wrapMode

    signal clicked

    implicitHeight: contentRow.height
    implicitWidth: contentRow.width

    opacity: root.enabled ? 1.0 : ui.theme.itemOpacityDisabled

    RowLayout {
        id: contentRow

        height: Math.max(box.height, label.implicitHeight)

        spacing: 8

        Rectangle {
            id: box

            height: 20
            width: 20

            border.width: 1
            border.color: "#00000000"

            radius: 2
            color: ui.theme.buttonColor
            opacity: ui.theme.buttonOpacityNormal

            StyledIconLabel {
                anchors.fill: parent
                iconCode: root.isIndeterminate ? IconCode.MINUS : IconCode.TICK_RIGHT_ANGLE

                visible: root.checked || root.isIndeterminate
            }
        }

        StyledTextLabel {
            id: label

            Layout.preferredWidth: root.width > 0 ? Math.min(root.width, label.implicitWidth) : label.implicitWidth
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
            maximumLineCount: 2

            visible: Boolean(text)
        }
    }

    MouseArea {
        id: clickableArea

        anchors.fill: contentRow
        anchors.margins: -4

        hoverEnabled: true

        onClicked: { root.clicked() }
    }

    states: [
        State {
            name: "HOVERED"
            when: clickableArea.containsMouse && !clickableArea.pressed

            PropertyChanges {
                target: box
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHover
                border.color: ui.theme.strokeColor
                border.width: 1
            }
        },

        State {
            name: "PRESSED"
            when: clickableArea.containsMouse && clickableArea.pressed

            PropertyChanges {
                target: box
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHit
                border.color: ui.theme.strokeColor
            }
        }
    ]
}
