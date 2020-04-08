import QtQuick 2.0

FocusableItem {
    id: root

    property bool checked: false
    property bool isIndeterminate: false

    property alias text: label.text

    signal clicked

    implicitHeight: contentRow.height
    width: contentRow.width

    opacity: root.enabled ? 1.0 : 0.3

    Row {
        id: contentRow

        height: Math.max(box.height, label.implicitHeight)
        width: box.width + spacing + label.implicitWidth

        spacing: 8

        Rectangle {
            id: box

            height: 20
            width: 20

            border.width: 1

            radius: 2
            color: Qt.rgba(globalStyle.button.r, globalStyle.button.g, globalStyle.button.b, 0.75)
            border.color: "#A2A2A2"

            Image {
                anchors.centerIn: parent

                sourceSize.width: 12
                sourceSize.height: 12

                source: root.isIndeterminate ? "qrc:/resources/icons/indeterminate_check_mark.svg" : "qrc:/resources/icons/check-mark.svg"

                visible: root.checked || root.isIndeterminate
            }
        }

        StyledTextLabel {
            id: label

            anchors.verticalCenter: box.verticalCenter
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
                radius: 1
                color: Qt.rgba(globalStyle.button.r, globalStyle.button.g, globalStyle.button.b, 1.0)
                border.color: Qt.rgba(0, 0, 0, 0.15)
            }
        },

        State {
            name: "PRESSED"
            when: clickableArea.containsMouse && clickableArea.pressed

            PropertyChanges {
                target: box
                radius: 1
                color: "#C0C0C0"
                border.color: Qt.rgba(0, 0, 0, 0.15)
            }
        }
    ]
}
