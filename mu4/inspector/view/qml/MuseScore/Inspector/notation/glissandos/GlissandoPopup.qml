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

        titleText: qsTr("Glissando line")
        propertyItem: root.model ? root.model.lineType : null

        RadioButtonGroup {
            id: radioButtonList

            height: 30
            width: parent.width

            model: [
                { textRole: qsTr("Straight"), valueRole: Glissando.LINE_TYPE_STRAIGHT },
                { textRole: qsTr("Wavy"), valueRole: Glissando.LINE_TYPE_WAVY }
            ]

            delegate: FlatRadioButton {
                id: radioButtonDelegate

                ButtonGroup.group: radioButtonList.radioButtonGroup

                checked: root.model && !root.model.lineType.isUndefined ? root.model.lineType.value === modelData["valueRole"]
                                                                        : false
                onToggled: {
                    root.model.lineType.value = modelData["valueRole"]
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
