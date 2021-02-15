import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
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
            titleText: qsTrc("inspector", "Bend type")
            propertyItem: root.model ? root.model.bendType : null

            StyledComboBox {
                width: parent.width

                textRoleName: "text"
                valueRoleName: "value"

                model: [
                    { text: qsTrc("inspector", "Bend"), value: BendTypes.TYPE_BEND },
                    { text: qsTrc("inspector", "Bend/Release"), value: BendTypes.TYPE_BEND_RELEASE },
                    { text: qsTrc("inspector", "Bend/Release/Bend"), value: BendTypes.TYPE_BEND_RELEASE_BEND },
                    { text: qsTrc("inspector", "Prebend"), value: BendTypes.TYPE_PREBEND },
                    { text: qsTrc("inspector", "Prebend/Release"), value: BendTypes.TYPE_PREBEND_RELEASE },
                    { text: qsTrc("inspector", "Custom"), value: BendTypes.TYPE_CUSTOM }
                ]

                currentIndex: root.model && !root.model.bendType.isUndefined ? indexOfValue(root.model.bendType.value) : -1

                onValueChanged: {
                    root.model.bendType.value = value
                }
            }
        }

        InspectorPropertyView {
            titleText: qsTrc("inspector", "Click to add or remove points")
            propertyItem: root.model ? root.model.bendCurve : null

            GridCanvas {
                height: 200
                width: parent.width

                pointList: root.model && root.model.bendCurve.isEnabled ? root.model.bendCurve.value : undefined

                rowCount: 13
                columnCount: 13
                rowSpacing: 4
                columnSpacing: 3

                onCanvasChanged: {
                    if (root.model) {
                        root.model.bendCurve.value = pointList
                    }
                }
            }
        }

        InspectorPropertyView {
            titleText: qsTrc("inspector", "Line thickness")
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
    }
}
