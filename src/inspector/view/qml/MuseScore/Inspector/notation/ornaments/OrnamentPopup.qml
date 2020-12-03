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

        spacing: 12

        InspectorPropertyView {

            titleText: qsTr("Performance")
            propertyItem: root.model ? root.model.performanceType : null

            RadioButtonGroup {
                id: radioButtonList

                height: 30
                width: parent.width

                model: [
                    { textRole: qsTr("Standard"), valueRole: OrnamentTypes.STYLE_STANDARD },
                    { textRole: qsTr("Baroque"), valueRole: OrnamentTypes.STYLE_BAROQUE }
                ]

                delegate: FlatRadioButton {
                    id: radioButtonDelegate

                    ButtonGroup.group: radioButtonList.radioButtonGroup

                    checked: root.model && !root.model.performanceType.isUndefined ? root.model.performanceType.value === modelData["valueRole"]
                                                                                   : false
                    onToggled: {
                        root.model.performanceType.value = modelData["valueRole"]
                    }

                    StyledTextLabel {
                        text: modelData["textRole"]

                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }

        InspectorPropertyView {
            titleText: qsTr("Placement")
            propertyItem: root.model ? root.model.placement : null

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
            width: parent.width

            text: qsTr("Channel & Midi properties")

            onClicked: {
                if (root.model) {
                    root.model.openChannelAndMidiProperties()
                }
            }
        }
    }
}
