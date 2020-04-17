import QtQuick 2.9
import "../../../common"

Item {
    id: root

    property QtObject leadingSpace: undefined
    property QtObject barWidth: undefined

    height: childrenRect.height
    width: parent.width

    Column {
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 4

        spacing: 8

        StyledTextLabel {
            text: qsTr("Leading")
        }

        IncrementalPropertyControl {
            icon: "qrc:/resources/icons/horizontal_adjustment.svg"

            enabled: leadingSpace ? leadingSpace.isEnabled : false
            isIndeterminate: leadingSpace && enabled ? leadingSpace.isUndefined : false
            currentValue: leadingSpace ? leadingSpace.value : 0

            onValueEdited: { leadingSpace.value = newValue }
        }
    }

    Column {
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: 4
        anchors.right: parent.right

        spacing: 8

        StyledTextLabel {
            text: qsTr("Bar width")
        }

        IncrementalPropertyControl {
            icon: "qrc:/resources/icons/horizontal_adjustment.svg"

            enabled: barWidth ? barWidth.isEnabled : false
            isIndeterminate: barWidth && enabled ? barWidth.isUndefined : false
            currentValue: barWidth ? barWidth.value : 0

            onValueEdited: { barWidth.value = newValue }
        }
    }
}
