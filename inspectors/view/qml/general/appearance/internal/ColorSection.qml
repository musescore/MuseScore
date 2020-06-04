import QtQuick 2.9
import QtQuick.Dialogs 1.2
import "../../../common"

Column {
    id: root

    property QtObject color: undefined

    height: implicitHeight
    width: parent.width

    spacing: 8

    StyledTextLabel {
        text: qsTr("Colour")
    }

    ColorPicker {
        id: colorPicker

        enabled: root.color ? root.color.isEnabled : false
        isIndeterminate: root.color && enabled ? root.color.isUndefined : false
        color: root.color && !root.color.isUndefined ? root.color.value : globalStyle.window

        onNewColorSelected: {
            if (root.color) {
                root.color.value = newColor
            }
        }
    }
}
