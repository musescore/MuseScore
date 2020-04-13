import QtQuick 2.9
import QtQuick.Controls 2.2
import "../../../common"

Column {
    id: root

    height: implicitHeight
    width: parent.width

    spacing: 16

    Column {
        width: parent.width

        spacing: 8

        StyledTextLabel {
            text: qsTr("Arrange")
        }

        RadioButtonGroup {
            id: radioButtonList

            height: 30
            width: parent.width

            model: [
                { textRole: "Backwards", valueRole: 1 },
                { textRole: "Forwards", valueRole: 2 }
            ]

            delegate: FlatRadioButton {
                id: radioButtonDelegate

                ButtonGroup.group: radioButtonList.radioButtonGroup

                StyledTextLabel {
                    text: modelData["textRole"]

                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }
    }

    Column {
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 2

        spacing: 8

        StyledTextLabel {
            text: qsTr("Arrange order")
        }

        IncrementalPropertyControl {
            iconMode: iconModeEnum.hidden
        }
    }
}
