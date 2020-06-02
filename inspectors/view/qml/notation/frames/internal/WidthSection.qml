import QtQuick 2.9
import MuseScore.Inspectors 3.3
import "../../../common"

Column {
    id: root

    property QtObject widthProperty: undefined

    anchors.left: parent.left
    anchors.right: parent.horizontalCenter
    anchors.rightMargin: 2

    spacing: 8

    StyledTextLabel {
        text: qsTr("Width")
    }

    IncrementalPropertyControl {
        icon: IconNameTypes.HORIZONTAL

        isIndeterminate: widthProperty ? widthProperty.isUndefined : false
        currentValue: widthProperty ? widthProperty.value : 0

        onValueEdited: { widthProperty.value = newValue }
    }
}
