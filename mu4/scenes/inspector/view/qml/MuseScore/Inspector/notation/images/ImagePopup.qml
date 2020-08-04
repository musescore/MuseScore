import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../common"

StyledPopup {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        Item {
            height: childrenRect.height
            width: parent.width

            InspectorPropertyView {
                id: heightControl

                anchors.left: parent.left
                anchors.right: lockButton.left
                anchors.rightMargin: 6

                titleText: qsTr("Image height")
                propertyItem: model ? model.height : null

                IncrementalPropertyControl {

                    icon: IconCode.VERTICAL
                    measureUnitsSymbol: staffSpaceUnitsCheckbox.checked ? qsTr("sp") : qsTr("mm")
                    isIndeterminate: model ? model.height.isUndefined : false
                    currentValue: model ? model.height.value : 0

                    onValueEdited: { model.height.value = newValue }
                }
            }

            FlatToogleButton {
                id: lockButton

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: heightControl.verticalCenter

                height: 20
                width: 20

                icon: checked ? IconCode.LOCK_CLOSED : IconCode.LOCK_OPEN

                checked: model ? model.isAspectRatioLocked.value : false

                onToggled: {
                    model.isAspectRatioLocked.value = !model.isAspectRatioLocked.value
                }
            }

            InspectorPropertyView {
                anchors.left: lockButton.right
                anchors.leftMargin: 6
                anchors.right: parent.right

                titleText: qsTr("Image width")
                propertyItem: model ? model.width : null

                IncrementalPropertyControl {
                    icon: IconCode.HORIZONTAL
                    iconMode: iconModeEnum.right
                    measureUnitsSymbol: staffSpaceUnitsCheckbox.checked ? qsTr("sp") : qsTr("mm")
                    isIndeterminate: model ? model.width.isUndefined : false
                    currentValue: model ? model.width.value : 0

                    onValueEdited: { model.width.value = newValue }
                }
            }
        }

        SeparatorLine { anchors.margins: -10 }

        CheckBox {
            enabled: model ? model.shouldScaleToFrameSize.isEnabled : false
            isIndeterminate: model ? model.shouldScaleToFrameSize.isUndefined : false
            checked: model && !isIndeterminate ? model.shouldScaleToFrameSize.value : false
            text: qsTr("Scale to frame size")

            onClicked: { model.shouldScaleToFrameSize.value = !checked }
        }

        CheckBox {
            id: staffSpaceUnitsCheckbox

            isIndeterminate: model ? model.isSizeInSpatiums.isUndefined : false
            checked: model && !isIndeterminate ? model.isSizeInSpatiums.value : false
            text: qsTr("Use staff space units")

            onClicked: { model.isSizeInSpatiums.value = !checked }
        }
    }
}
