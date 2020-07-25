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

        Column {
            spacing: 8

            height: childrenRect.height
            width: parent.width

            Item {
                height: childrenRect.height
                width: parent.width


                InspectorPropertyView {
                    titleText: qsTr("Number visible")
                    propertyItem: root.model ? root.model.isNumberVisible : null

                    anchors.left: parent.left
                    anchors.right: parent.horizontalCenter
                    anchors.rightMargin: 2

                    spacing: 8

                    CheckBox {
                        id: numberVisibilityCheckBox

                        isIndeterminate: root.model ? root.model.isNumberVisible.isUndefined : false
                        checked: root.model && !isIndeterminate ? root.model.isNumberVisible.value : false
                        onClicked: { root.model.isNumberVisible.value = !checked }
                    }
                }

                InspectorPropertyView {
                    titleText: qsTr("Number position")
                    propertyItem: root.model ? root.model.numberPosition : null

                    anchors.left: parent.horizontalCenter
                    anchors.leftMargin: 2
                    anchors.right: parent.right

                    spacing: 8

                    IncrementalPropertyControl {
                        id: numberPositionControl

                        icon: IconCode.VERTICAL
                        enabled: root.model ? model.isEmpty || numberVisibilityCheckBox.checked : true
                        isIndeterminate: root.model ? root.model.numberPosition.isUndefined : false
                        currentValue: root.model ? root.model.numberPosition.value : 0

                        step: 0.5
                        decimals: 2
                        maxValue: 99.00
                        minValue: -99.00

                        onValueEdited: { root.model.numberPosition.value = newValue }
                    }
                }
            }
        }
    }
}
