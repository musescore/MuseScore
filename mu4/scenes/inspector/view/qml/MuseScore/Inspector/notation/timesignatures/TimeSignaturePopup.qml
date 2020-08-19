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

        InspectorPropertyView {
            height: childrenRect.height

            titleText: qsTr("Scale")
            propertyItem: root.model ? root.model.horizontalScale : null

            Item {
                height: childrenRect.height
                width: parent.width

                IncrementalPropertyControl {
                    id: horizontalScaleControl

                    anchors.left: parent.left
                    anchors.right: parent.horizontalCenter
                    anchors.rightMargin: 2

                    icon: IconCode.HORIZONTAL
                    isIndeterminate: root.model ? root.model.horizontalScale.isUndefined : false
                    currentValue: root.model ? root.model.horizontalScale.value : 0

                    measureUnitsSymbol: "%"
                    step: 1
                    decimals: 0
                    maxValue: 300
                    minValue: 1
                    validator: IntInputValidator {
                        top: horizontalScaleControl.maxValue
                        bottom: horizontalScaleControl.minValue
                    }

                    onValueEdited: { root.model.horizontalScale.value = newValue }
                }

                IncrementalPropertyControl {
                    id: verticalScaleControl

                    anchors.left: parent.horizontalCenter
                    anchors.leftMargin: 2
                    anchors.right: parent.right

                    icon: IconCode.VERTICAL
                    isIndeterminate: root.model ? root.model.verticalScale.isUndefined : false
                    currentValue: root.model ? root.model.verticalScale.value : 0

                    measureUnitsSymbol: "%"
                    step: 1
                    decimals: 0
                    maxValue: 300
                    minValue: 1
                    validator: IntInputValidator {
                        top: verticalScaleControl.maxValue
                        bottom: verticalScaleControl.minValue
                    }

                    onValueEdited: { root.model.verticalScale.value = newValue }
                }
            }
        }

        CheckBox {
            isIndeterminate: model ? model.shouldShowCourtesy.isUndefined : false
            checked: model && !isIndeterminate ? model.shouldShowCourtesy.value : false
            text: qsTr("Show courtesy time signature on previous system")

            onClicked: { model.shouldShowCourtesy.value = !checked }
        }

        FlatButton {
            width: parent.width

            text: qsTr("Change time signature")

            onClicked: {
                if (root.model) {
                    root.model.showTimeSignatureProperties()
                }
            }
        }
    }
}
