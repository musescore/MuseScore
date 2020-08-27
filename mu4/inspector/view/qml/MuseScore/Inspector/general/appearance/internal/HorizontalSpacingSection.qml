import QtQuick 2.9
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../../common"

Item {
    id: root

    property QtObject leadingSpace: undefined
    property QtObject barWidth: undefined

    height: childrenRect.height
    width: parent.width

    InspectorPropertyView {
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 4

        titleText: qsTr("Leading")
        propertyItem: leadingSpace

        IncrementalPropertyControl {
            icon: IconCode.HORIZONTAL

            enabled: leadingSpace ? leadingSpace.isEnabled : false
            isIndeterminate: leadingSpace && enabled ? leadingSpace.isUndefined : false
            currentValue: leadingSpace ? leadingSpace.value : 0

            onValueEdited: { leadingSpace.value = newValue }
        }
    }

    InspectorPropertyView {
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: 4
        anchors.right: parent.right

        titleText: qsTr("Bar width")
        propertyItem: barWidth

        IncrementalPropertyControl {
            icon: IconCode.HORIZONTAL

            enabled: barWidth ? barWidth.isEnabled : false
            isIndeterminate: barWidth && enabled ? barWidth.isUndefined : false
            currentValue: barWidth ? barWidth.value : 0

            onValueEdited: { barWidth.value = newValue }
        }
    }
}
