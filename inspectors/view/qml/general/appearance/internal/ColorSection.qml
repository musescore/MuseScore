import QtQuick 2.9
import QtQuick.Dialogs 1.2
import "../../../common"

Column {
    id: root

    property alias color: colorPicker.color

    height: implicitHeight
    width: parent.width

    spacing: 8

    StyledTextLabel {
        text: qsTr("Colour")
    }

    ColorPicker {
        id: colorPicker
    }
}
