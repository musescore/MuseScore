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
            titleText: qsTr("Jump to")
            propertyItem: model ? model.jumpTo : null

            TextInputField {
                isIndeterminate: model ? model.jumpTo.isUndefined : false
                currentText: model ? model.jumpTo.value : ""
                enabled: model ? model.jumpTo.isEnabled : false

                onCurrentTextEdited: {
                    if (!root.model) {
                        return
                    }

                    root.model.jumpTo.value = newTextValue
                }
            }
        }

        InspectorPropertyView {

            titleText: qsTr("Play until")
            propertyItem: model ? model.playUntil : null

            TextInputField {
                isIndeterminate: model ? model.playUntil.isUndefined : false
                currentText: model ? model.playUntil.value : ""
                enabled: model ? model.playUntil.isEnabled : false

                onCurrentTextEdited: {
                    if (!root.model) {
                        return
                    }

                    root.model.playUntil.value = newTextValue
                }
            }
        }

        InspectorPropertyView {
            titleText: qsTr("Continue at")
            propertyItem: model ? model.continueAt : null

            TextInputField {
                isIndeterminate: model ? model.continueAt.isUndefined : false
                currentText: model ? model.continueAt.value : ""
                enabled: model ? model.continueAt.isEnabled : false
            }
        }

        CheckBox {
            isIndeterminate: model ? model.hasToPlayRepeats.isUndefined : false
            checked: model && !isIndeterminate ? model.hasToPlayRepeats.value : false
            text: qsTr("Play repeats")

            onClicked: { model.hasToPlayRepeats.value = !checked }
        }
    }
}

