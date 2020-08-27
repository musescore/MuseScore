import QtQuick 2.9
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../../common"

InspectorPropertyView {
    id: root

    property QtObject heightProperty: undefined

    anchors.left: parent.left
    anchors.right: parent.horizontalCenter
    anchors.rightMargin: 2

    titleText: qsTr("Height")
    propertyItem: heightProperty

    IncrementalPropertyControl {
        icon: IconCode.VERTICAL

        isIndeterminate: heightProperty ? heightProperty.isUndefined : false
        currentValue: heightProperty ? heightProperty.value : 0

        onValueEdited: { heightProperty.value = newValue }
    }
}
