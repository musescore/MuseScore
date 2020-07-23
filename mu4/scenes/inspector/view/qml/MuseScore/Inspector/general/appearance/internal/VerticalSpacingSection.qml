import QtQuick 2.9
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../../common"

InspectorPropertyView {
    id: root

    property QtObject minimumDistance: undefined

    anchors.left: parent.left
    anchors.right: parent.horizontalCenter
    anchors.rightMargin: 2

    titleText: qsTr("Minimum distance")
    propertyItem: minimumDistance

    IncrementalPropertyControl {
        icon: IconCode.VERTICAL

        isIndeterminate: minimumDistance ? minimumDistance.isUndefined : false
        currentValue: minimumDistance ? minimumDistance.value : 0

        onValueEdited: { minimumDistance.value = newValue }
    }
}
