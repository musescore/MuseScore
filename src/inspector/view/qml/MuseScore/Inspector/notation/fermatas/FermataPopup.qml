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

    InspectorPropertyView {
        id: contentColumn

        titleText: qsTr("Placement on staff")
        propertyItem: root.model ? root.model.placementType : null

        RadioButtonGroup {
            id: radioButtonList

            height: 30
            width: parent.width

            model: [
                { textRole: "Above", valueRole: FermataTypes.ABOVE },
                { textRole: "Below", valueRole: FermataTypes.BELOW }
            ]

            delegate: FlatRadioButton {
                id: radioButtonDelegate

                ButtonGroup.group: radioButtonList.radioButtonGroup

                checked: root.model && !root.model.placementType.isUndefined ? root.model.placementType.value === modelData["valueRole"]
                                                                             : false
                onToggled: {
                    root.model.placementType.value = modelData["valueRole"]
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
}
