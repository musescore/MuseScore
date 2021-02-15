import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import "../../common"

StyledPopup {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 16

        InspectorPropertyView {
            titleText: qsTrc("inspector", "Interpretation")
            propertyItem: root.model ? root.model.isLiteral : null

            RadioButtonGroup {
                id: interpretationTypeList

                height: 30
                width: parent.width

                model: [
                    { textRole: qsTrc("inspector", "Literal"), valueRole: true },
                    { textRole: qsTrc("inspector", "Jazz"), valueRole: false }
                ]

                delegate: FlatRadioButton {

                    ButtonGroup.group: interpretationTypeList.radioButtonGroup

                    checked: root.model && !root.model.isLiteral.isUndefined ? root.model.isLiteral.value === modelData["valueRole"]
                                                                             : false

                    onToggled: {
                        root.model.isLiteral.value = modelData["valueRole"]
                    }

                    StyledTextLabel {
                        text: modelData["textRole"]
                    }
                }
            }
        }

        InspectorPropertyView {
            titleText: qsTrc("inspector", "Voicing")
            propertyItem: root.model ? root.model.voicingType : null

            StyledComboBox {
                width: parent.width

                textRoleName: "text"
                valueRoleName: "value"

                model: [
                    { text: qsTrc("inspector", "Auto"), value: ChordSymbolTypes.VOICING_AUTO },
                    { text: qsTrc("inspector", "Root only"), value: ChordSymbolTypes.VOICING_ROOT_ONLY },
                    { text: qsTrc("inspector", "Close"), value: ChordSymbolTypes.VOICING_CLOSE },
                    { text: qsTrc("inspector", "Drop two"), value: ChordSymbolTypes.VOICING_DROP_TWO },
                    { text: qsTrc("inspector", "Six note"), value: ChordSymbolTypes.VOICING_SIX_NOTE },
                    { text: qsTrc("inspector", "Four note"), value: ChordSymbolTypes.VOICING_FOUR_NOTE },
                    { text: qsTrc("inspector", "Three note"), value: ChordSymbolTypes.VOICING_THREE_NOTE }
                ]

                currentIndex: root.model && !root.model.voicingType.isUndefined ? indexOfValue(root.model.voicingType.value) : -1

                onValueChanged: {
                    root.model.voicingType.value = value
                }
            }
        }

        InspectorPropertyView {
            titleText: qsTrc("inspector", "Duration")
            propertyItem: root.model ? root.model.durationType : null

            StyledComboBox {
                width: parent.width

                textRoleName: "text"
                valueRoleName: "value"

                model: [
                    { text: qsTrc("inspector", "Until the next chord symbol"), value: ChordSymbolTypes.DURATION_UNTIL_NEXT_CHORD_SYMBOL },
                    { text: qsTrc("inspector", "Until the end of the bar"), value: ChordSymbolTypes.DURATION_STOP_AT_MEASURE_END },
                    { text: qsTrc("inspector", "Until the end of the attached duration"), value: ChordSymbolTypes.DURATION_SEGMENT_DURATION }
                ]

                currentIndex: root.model && !root.model.durationType.isUndefined ? indexOfValue(root.model.durationType.value) : -1

                onValueChanged: {
                    root.model.durationType.value = value
                }
            }
        }
    }
}
