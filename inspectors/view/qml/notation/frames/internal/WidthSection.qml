import QtQuick 2.9
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
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
        icon: IconCode.HORIZONTAL

        isIndeterminate: widthProperty ? widthProperty.isUndefined : false
        currentValue: widthProperty ? widthProperty.value : 0

        onValueEdited: { widthProperty.value = newValue }
    }
}
