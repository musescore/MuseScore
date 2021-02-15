import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
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

        InspectorPropertyView {

            titleText: qsTrc("inspector", "Direction")
            propertyItem: root.model ? root.model.direction : null

            RadioButtonGroup {
                id: radioButtonList

                height: 30
                width: parent.width

                model: [
                    { iconRole: IconCode.AUTO, typeRole: ArticulationTypes.AUTO },
                    { iconRole: IconCode.ARROW_DOWN, typeRole: ArticulationTypes.DOWN },
                    { iconRole: IconCode.ARROW_UP, typeRole: ArticulationTypes.UP }
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
        }

        InspectorPropertyView {
            titleText: qsTrc("inspector", "Placement")
            propertyItem: root.model ? root.model.placement : null

            StyledComboBox {
                width: parent.width

                textRoleName: "text"
                valueRoleName: "value"

                model: [
                    { text: qsTrc("inspector", "Above staff"), value: ArticulationTypes.TYPE_ABOVE_STAFF },
                    { text: qsTrc("inspector", "Below staff"), value: ArticulationTypes.TYPE_BELOW_STAFF },
                    { text: qsTrc("inspector", "Chord automatic"), value: ArticulationTypes.TYPE_CHORD_AUTO },
                    { text: qsTrc("inspector", "Above chord"), value: ArticulationTypes.TYPE_ABOVE_CHORD },
                    { text: qsTrc("inspector", "Below chord"), value: ArticulationTypes.TYPE_BELOW_CHORD }
                ]

                currentIndex: root.model && !root.model.placement.isUndefined ? indexOfValue(root.model.placement.value) : -1

                onValueChanged: {
                    root.model.placement.value = value
                }
            }
        }

        FlatButton {
            width: parent.width

            text: qsTrc("inspector", "Channel & Midi properties")

            onClicked: {
                root.model.openChannelAndMidiProperties()
            }
        }
    }
}
