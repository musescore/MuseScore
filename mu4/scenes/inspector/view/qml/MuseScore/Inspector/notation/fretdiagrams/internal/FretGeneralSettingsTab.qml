import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../../common"

FocusableItem {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        height: childrenRect.height
        width: parent.width

        spacing: 16

        visible: root.model ? root.model.areSettingsAvailable : false

        Item {
            height: childrenRect.height
            width: parent.width

            CheckBox {
                id: barreModeCheckBox

                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                enabled: root.model ? !root.model.isMultipleDotsModeOn : false
                checked: root.model && enabled ? root.model.isBarreModeOn : false
                text: qsTr("Barre")

                onClicked: { root.model.isBarreModeOn = !checked }
            }

            CheckBox {
                id: multipleDotsModeCheckBox

                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                enabled: root.model ? !root.model.isBarreModeOn : false
                checked: root.model && enabled ? root.model.isMultipleDotsModeOn : false
                text: qsTr("Multiple dots")

                onClicked: { root.model.isMultipleDotsModeOn = !checked }
            }
        }

        Column {
            width: parent.width

            spacing: 8

            StyledTextLabel {
                text: qsTr("Marker type")
            }

            RadioButtonGroup {
                id: lineStyleButtonList

                height: 30
                width: parent.width

                enabled: root.model ? !root.model.isBarreModeOn : false

                model: [
                    { iconRole: IconCode.FRETBOARD_MARKER_CIRCLE_FILLED, typeRole: FretDiagramTypes.DOT_NORMAL },
                    { iconRole: IconCode.CLOSE_X_ROUNDED, typeRole: FretDiagramTypes.DOT_CROSS },
                    { iconRole: IconCode.STOP, typeRole: FretDiagramTypes.DOT_SQUARE },
                    { iconRole: IconCode.FRETBOARD_MARKER_TRIANGLE, typeRole: FretDiagramTypes.DOT_TRIANGLE }
                ]

                delegate: FlatRadioButton {

                    ButtonGroup.group: lineStyleButtonList.radioButtonGroup

                    checked: root.model ? root.model.currentFretDotType === modelData["typeRole"] : false

                    onToggled: {
                        root.model.currentFretDotType = modelData["typeRole"]
                    }

                    StyledIconLabel {
                        iconCode: modelData["iconRole"]
                    }
                }
            }
        }
    }
}

