import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspectors 3.3
import "../../common"

StyledPopup {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        StyledTextLabel {
            text: qsTr("Direction")
        }

        RadioButtonGroup {
            id: radioButtonList

            height: 30
            width: parent.width

            model: [
                { iconRole: IconNameTypes.AUTO, typeRole: Articulation.AUTO },
                { iconRole: IconNameTypes.ARROW_DOWN, typeRole: Articulation.DOWN },
                { iconRole: IconNameTypes.ARROW_UP, typeRole: Articulation.UP }
            ]

            delegate: FlatRadioButton {

                ButtonGroup.group: radioButtonList.radioButtonGroup

                checked: root.model && !root.model.direction.isUndefined ? root.model.direction.value === modelData["typeRole"]
                                                                                     : false

                onToggled: {
                    root.model.direction.value = modelData["typeRole"]
                }

                StyledIconLabel {
                    iconCode: modelData["iconRole"]
                }
            }
        }

        Column {
            spacing: 8

            width: parent.width

            StyledTextLabel {
                text: qsTr("Placement")
            }

            StyledComboBox {
                width: parent.width

                textRoleName: "text"
                valueRoleName: "value"

                model: [
                    { text: qsTr("Above staff"), value: ArticulationTypes.TYPE_ABOVE_STAFF },
                    { text: qsTr("Below staff"), value: ArticulationTypes.TYPE_BELOW_STAFF },
                    { text: qsTr("Chord automatic"), value: ArticulationTypes.TYPE_CHORD_AUTO },
                    { text: qsTr("Above chord"), value: ArticulationTypes.TYPE_ABOVE_CHORD },
                    { text: qsTr("Below chord"), value: ArticulationTypes.TYPE_BELOW_CHORD }
                ]

                currentIndex: root.model && !root.model.placement.isUndefined ? indexOfValue(root.model.placement.value) : -1

                onValueChanged: {
                    root.model.placement.value = value
                }
            }
        }

        FlatButton {
            id: propertiesButton

            width: parent.width
            text: qsTr("Channel & Midi properties")

            onClicked: {
            }
        }
    }
}
