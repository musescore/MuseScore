import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspectors 3.3
import "../../../common"

FocusableItem {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        Column {
            width: parent.width
            spacing: 8

            StyledTextLabel {
                text: qsTr("Text when continuing to a new system")
            }

            TextInputField {
                isIndeterminate: root.model ? root.model.continiousText.isUndefined : false
                currentText: root.model ? root.model.continiousText.value : ""
                enabled: root.model ? root.model.continiousText.isEnabled : false

                onCurrentTextEdited: {
                    if (!root.model) {
                        return
                    }

                    root.model.continiousText.value = newTextValue
                }
            }
        }

        Column {
            spacing: 8

            height: childrenRect.height
            width: parent.width

            StyledTextLabel {
                anchors.left: parent.left

                text: qsTr("Offset")
            }

            Item {
                height: childrenRect.height
                width: parent.width

                IncrementalPropertyControl {
                    anchors.left: parent.left
                    anchors.right: parent.horizontalCenter
                    anchors.rightMargin: 2

                    icon: IconNameTypes.HORIZONTAL
                    isIndeterminate: root.model ? root.model.continiousTextHorizontalOffset.isUndefined : false
                    currentValue: root.model ? root.model.continiousTextHorizontalOffset.value : 0

                    onValueEdited: { root.model.continiousTextHorizontalOffset.value = newValue }
                }

                IncrementalPropertyControl {
                    anchors.left: parent.horizontalCenter
                    anchors.leftMargin: 2
                    anchors.right: parent.right

                    icon: IconNameTypes.VERTICAL
                    isIndeterminate: root.model ? root.model.continiousTextVerticalOffset.isUndefined : false
                    currentValue: root.model ? root.model.continiousTextVerticalOffset.value : 0

                    onValueEdited: { root.model.continiousTextVerticalOffset.value = newValue }
                }
            }
        }
    }
}

