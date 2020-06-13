import QtQuick 2.9
import MuseScore.UiComponents 1.0
import "../../../common"

ExpandableBlank {
    id: root

    property QtObject model: null

    enabled: model ? !model.isEmpty : false

    title: model ? model.title : ""

    width: parent.width

    contentItemComponent: Column {
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 2

        spacing: 8

        height: implicitHeight
        width: root.width

        StyledTextLabel {
            text: qsTr("Spread delay")
        }

        IncrementalPropertyControl {
            iconMode: iconModeEnum.hidden

            isIndeterminate: model ? model.stretch.isUndefined : false
            currentValue: model ? model.stretch.value : 0

            maxValue: 100
            minValue: 0

            onValueEdited: { model.stretch.value = newValue }
        }
    }
}
