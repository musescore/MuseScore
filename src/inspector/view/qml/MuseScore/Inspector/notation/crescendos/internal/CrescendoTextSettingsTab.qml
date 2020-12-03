import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
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

        InspectorPropertyView {
            titleText: qsTr("Beginning text")
            propertyItem: root.model ? root.model.beginningText : null

            TextInputField {
                isIndeterminate: root.model ? root.model.beginningText.isUndefined : false
                currentText: root.model ? root.model.beginningText.value : ""
                enabled: root.model ? root.model.beginningText.isEnabled : false

                onCurrentTextEdited: {
                    if (!root.model) {
                        return
                    }

                    root.model.beginningText.value = newTextValue
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

                    icon: IconCode.HORIZONTAL
                    isIndeterminate: root.model ? root.model.beginningTextHorizontalOffset.isUndefined : false
                    currentValue: root.model ? root.model.beginningTextHorizontalOffset.value : 0

                    onValueEdited: { root.model.beginningTextHorizontalOffset.value = newValue }
                }

                IncrementalPropertyControl {
                    anchors.left: parent.horizontalCenter
                    anchors.leftMargin: 2
                    anchors.right: parent.right

                    icon: IconCode.VERTICAL
                    isIndeterminate: root.model ? root.model.beginningTextVerticalOffset.isUndefined : false
                    currentValue: root.model ? root.model.beginningTextVerticalOffset.value : 0

                    onValueEdited: { root.model.beginningTextVerticalOffset.value = newValue }
                }
            }
        }

        SeparatorLine { anchors.margins: -10 }

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

                    icon: IconCode.HORIZONTAL
                    isIndeterminate: root.model ? root.model.continiousTextHorizontalOffset.isUndefined : false
                    currentValue: root.model ? root.model.continiousTextHorizontalOffset.value : 0

                    onValueEdited: { root.model.continiousTextHorizontalOffset.value = newValue }
                }

                IncrementalPropertyControl {
                    anchors.left: parent.horizontalCenter
                    anchors.leftMargin: 2
                    anchors.right: parent.right

                    icon: IconCode.VERTICAL
                    isIndeterminate: root.model ? root.model.continiousTextVerticalOffset.isUndefined : false
                    currentValue: root.model ? root.model.continiousTextVerticalOffset.value : 0

                    onValueEdited: { root.model.continiousTextVerticalOffset.value = newValue }
                }
            }
        }
    }
}

