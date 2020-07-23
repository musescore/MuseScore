import QtQuick 2.0
import QtQuick.Controls 2.15
import MuseScore.Ui 1.0

Rectangle {
    id: root

    signal searchRequested(var text)

    width: 184
    height: 30

    QtObject {
        id: privateProperties

        readonly property string borderColor: ui.theme.buttonColor
        readonly property string backgroundColor: ui.theme.backgroundColor
    }

    color: privateProperties.backgroundColor

    border.color: privateProperties.borderColor
    border.width: 1
    radius: 3

    Row {
        anchors.fill: parent

        Item {
            width: 32
            height: 30

            StyledIconLabel {
                anchors.centerIn: parent
                iconCode: IconCode.SEARCH
                color: privateProperties.borderColor
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    root.searchRequested(textField.text)
                }
            }
        }

        TextField {
            id: textField

            anchors.verticalCenter: parent.verticalCenter
            width: 134

            placeholderText: qsTrc("uicomponents", "Search")
            color: privateProperties.borderColor

            font.italic: true
            font.family: ui.theme.font.family
            font.pointSize: ui.theme.font.pointSize

            background: Rectangle {
                color: privateProperties.backgroundColor
            }

            selectByMouse: true

            Keys.onShortcutOverride: {
                event.accepted = (event.key === Qt.Key_Return)

                if (event.accepted) {
                    root.searchRequested(textField.text)
                }
            }
        }
    }
}
