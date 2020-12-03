import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import "../../common"

StyledPopup {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.height + topPadding + bottomPadding
    width: parent.width

    Column {
        id: contentColumn

        height: childrenRect.height
        width: parent.width

        spacing: 12

        InspectorPropertyView {
            visible: root.model ? root.model.areSettingsAvailable : false

            titleText: qsTr("Tremolo bar type")
            propertyItem: root.model ? root.model.type : null

            StyledComboBox {
                width: parent.width

                textRoleName: "text"
                valueRoleName: "value"

                model: [
                    { text: qsTr("Dip"), value: TremoloBarTypes.TYPE_DIP },
                    { text: qsTr("Dive"), value: TremoloBarTypes.TYPE_DIVE },
                    { text: qsTr("Release (Up)"), value: TremoloBarTypes.TYPE_RELEASE_UP },
                    { text: qsTr("Inverted dip"), value: TremoloBarTypes.TYPE_INVERTED_DIP },
                    { text: qsTr("Return"), value: TremoloBarTypes.TYPE_RETURN },
                    { text: qsTr("Release (Down)"), value: TremoloBarTypes.TYPE_RELEASE_DOWN },
                    { text: qsTr("Custom"), value: TremoloBarTypes.TYPE_CUSTOM }
                ]

                currentIndex: root.model && !root.model.type.isUndefined ? indexOfValue(root.model.type.value) : -1

                onValueChanged: {
                    root.model.type.value = value
                }
            }
        }

        InspectorPropertyView {

            visible: root.model ? root.model.areSettingsAvailable : false

            titleText: qsTr("Click to add or remove points")
            propertyItem: root.model ? root.model.curve : null

            GridCanvas {
                height: 300
                width: parent.width

                pointList: root.model && root.model.curve.isEnabled ? root.model.curve.value : []

                rowCount: 33
                columnCount: 13
                rowSpacing: 8
                columnSpacing: 3
                shouldShowNegativeRows: true

                onCanvasChanged: {
                    if (root.model) {
                        root.model.curve.value = pointList
                    }
                }
            }
        }

        Item {
            height: childrenRect.height
            width: parent.width

            visible: root.model ? root.model.areSettingsAvailable : false

            InspectorPropertyView {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                titleText: qsTr("Line thickness")
                propertyItem: model ? model.lineThickness : null

                IncrementalPropertyControl {
                    isIndeterminate: model ? model.lineThickness.isUndefined : false
                    currentValue: model ? model.lineThickness.value : 0
                    iconMode: iconModeEnum.hidden

                    maxValue: 10
                    minValue: 0.1
                    step: 0.1
                    decimals: 2

                    onValueEdited: { model.lineThickness.value = newValue }
                }
            }

            InspectorPropertyView {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                titleText: qsTr("Scale")
                propertyItem: model ? model.scale : null

                IncrementalPropertyControl {
                    isIndeterminate: model ? model.scale.isUndefined : false
                    currentValue: model ? model.scale.value : 0
                    iconMode: iconModeEnum.hidden

                    maxValue: 5
                    minValue: 0.1
                    step: 0.1
                    decimals: 2

                    onValueEdited: { model.scale.value = newValue }
                }
            }
        }
    }

    StyledTextLabel {
        anchors.fill: parent

        wrapMode: Text.Wrap
        text: qsTr("You have multiple tremolo bars selected. Select a single one to edit its settings")
        visible: root.model ? !root.model.areSettingsAvailable : false
    }
}
