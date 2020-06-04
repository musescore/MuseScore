import QtQuick 2.9
import MuseScore.Inspectors 3.3
import "../../../common"

Column {
    id: root

    property QtObject heightProperty: undefined

    anchors.left: parent.left
    anchors.right: parent.horizontalCenter
    anchors.rightMargin: 2

    spacing: 8

    StyledTextLabel {
        text: qsTr("Height")
    }

    IncrementalPropertyControl {
        icon: IconNameTypes.VERTICAL

        isIndeterminate: heightProperty ? heightProperty.isUndefined : false
        currentValue: heightProperty ? heightProperty.value : 0

        onValueEdited: { heightProperty.value = newValue }
    }
}
