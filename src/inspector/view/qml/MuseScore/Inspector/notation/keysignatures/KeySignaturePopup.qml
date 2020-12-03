import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import QtQuick.Layouts 1.3
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

        CheckBox {
            isIndeterminate: model ? model.hasToShowCourtesy.isUndefined : false
            checked: model && !isIndeterminate ? model.hasToShowCourtesy.value : false
            text: qsTr("Show courtesy key signature on previous system")

            onClicked: { model.hasToShowCourtesy.value = !checked }
        }

        InspectorPropertyView {

            titleText: qsTr("Mode")
            propertyItem: root.model ? root.model.mode : null

            StyledComboBox {
                width: parent.width

                textRoleName: "text"
                valueRoleName: "value"

                model: [
                    { text: qsTr("Unknown"), value: KeySignatureTypes.MODE_UNKNOWN },
                    { text: qsTr("None"), value: KeySignatureTypes.MODE_NONE },
                    { text: qsTr("Major"), value: KeySignatureTypes.MODE_MAJOR },
                    { text: qsTr("Minor"), value: KeySignatureTypes.MODE_MINOR },
                    { text: qsTr("Dorian"), value: KeySignatureTypes.MODE_DORIAN },
                    { text: qsTr("Phrygian"), value: KeySignatureTypes.MODE_PHRYGIAN },
                    { text: qsTr("Lydian"), value: KeySignatureTypes.MODE_LYDIAN },
                    { text: qsTr("Mixolydian"), value: KeySignatureTypes.MODE_MIXOLYDIAN },
                    { text: qsTr("Ionian"), value: KeySignatureTypes.MODE_IONIAN },
                    { text: qsTr("Locrian"), value: KeySignatureTypes.MODE_LOCRIAN }
                ]

                currentIndex: root.model && !root.model.mode.isUndefined ? indexOfValue(root.model.mode.value) : -1

                onValueChanged: {
                    root.model.mode.value = value
                }
            }
        }
    }
}
