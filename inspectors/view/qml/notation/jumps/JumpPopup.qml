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

        Column {
            width: parent.width
            spacing: 8

            StyledTextLabel {
                text: qsTr("Jump to")
            }

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

        Column {
            width: parent.width
            spacing: 8

            StyledTextLabel {
                text: qsTr("Play until")
            }

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

        Column {
            width: parent.width
            spacing: 8

            StyledTextLabel {
                text: qsTr("Continue at")
            }

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

