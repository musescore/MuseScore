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

        titleText: qsTrc("inspector", "Bracket type")
        propertyItem: root.model ? root.model.bracketType : null

        RadioButtonGroup {
            id: radioButtonList

            height: 30
            width: parent.width

            model: [
                { textRole: qsTrc("inspector", "None"), valueRole: AccidentalTypes.BRACKET_TYPE_NONE },
                { textRole: qsTrc("inspector", "Parenthesis"), valueRole: AccidentalTypes.BRACKET_TYPE_PARENTHESIS },
                { textRole: qsTrc("inspector", "Bracket"), valueRole: AccidentalTypes.BRACKET_TYPE_SQUARE },
                { textRole: qsTrc("inspector", "Brace"), valueRole: AccidentalTypes.BRACKET_TYPE_ROUND },
            ]

            delegate: FlatRadioButton {
                id: radioButtonDelegate

                ButtonGroup.group: radioButtonList.radioButtonGroup

                checked: root.model && !root.model.bracketType.isUndefined ? root.model.bracketType.value === modelData["valueRole"]
                                                                           : false
                onToggled: {
                    root.model.bracketType.value = modelData["valueRole"]
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
