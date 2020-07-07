import QtQuick 2.9
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../../common"

Item {
    id: root

    property QtObject frameLeftMargin: undefined
    property QtObject frameRightMargin: undefined

    height: childrenRect.height
    width: parent.width

    InspectorPropertyView {
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 4

        titleText: qsTr("Left margin")
        propertyItem: frameLeftMargin

        IncrementalPropertyControl {
            icon: IconCode.LEFT_MARGIN

            measureUnitsSymbol: qsTr("mm")

            enabled: frameLeftMargin ? frameLeftMargin.isEnabled : false
            isIndeterminate: frameLeftMargin && enabled ? frameLeftMargin.isUndefined : false
            currentValue: frameLeftMargin ? frameLeftMargin.value : 0

            onValueEdited: { frameLeftMargin.value = newValue }
        }
    }

    InspectorPropertyView {
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: 4
        anchors.right: parent.right

        titleText: qsTr("Right margin")
        propertyItem: frameRightMargin

        IncrementalPropertyControl {
            icon: IconCode.RIGHT_MARGIN

            measureUnitsSymbol: qsTr("mm")

            enabled: frameRightMargin ? frameRightMargin.isEnabled : false
            isIndeterminate: frameRightMargin && enabled ? frameRightMargin.isUndefined : false
            currentValue: frameRightMargin ? frameRightMargin.value : 0

            onValueEdited: { frameRightMargin.value = newValue }
        }
    }
}
