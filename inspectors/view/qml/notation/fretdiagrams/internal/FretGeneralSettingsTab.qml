import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspectors 3.3
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
                    { iconRole: IconNameTypes.FRETBOARD_MARKER_CIRCLE_FILLED, typeRole: FretDiagramTypes.DOT_NORMAL },
                    { iconRole: IconNameTypes.CLOSE_X_ROUNDED, typeRole: FretDiagramTypes.DOT_CROSS },
                    { iconRole: IconNameTypes.STOP, typeRole: FretDiagramTypes.DOT_SQUARE },
                    { iconRole: IconNameTypes.FRETBOARD_MARKER_TRIANGLE, typeRole: FretDiagramTypes.DOT_TRIANGLE }
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

