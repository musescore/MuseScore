import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import MuseScore.Inspectors 3.3
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../common"

StyledPopup {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        RowLayout {

            width: parent.width

            spacing: 4

            IncrementalPropertyControl {
                id: measureCountControl

                Layout.fillWidth: true
                Layout.preferredWidth: parent.width * 0.25

                iconMode: iconModeEnum.hidden

                currentValue: model ? model.measureCount : 0

                step: 1
                decimals: 0
                maxValue: 10
                minValue: 0
                validator: IntInputValidator {
                    top: measureCountControl.maxValue
                    bottom: measureCountControl.minValue
                }

                onValueEdited: { model.measureCount = newValue }
            }

            StyledComboBox {
                Layout.fillWidth: true
                Layout.preferredWidth: parent.width * 0.75

                textRoleName: "text"
                valueRoleName: "value"

                model: updateModel(root.model ? !root.model.isEmpty : false)

                currentIndex: root.model ? indexOfValue(root.model.measureInsertionType) : -1

                onValueChanged: {
                    root.model.measureInsertionType = value
                }

                function updateModel(hasAnySelection) {
                    var result = [
                                   { text: qsTr("At start of score"), value: MeasureTypes.TYPE_PREPEND_TO_SCORE },
                                   { text: qsTr("At end of score"), value: MeasureTypes.TYPE_APPEND_TO_SCORE }
                                 ]

                    if (hasAnySelection) {
                        result.push({ text: qsTr("Before selection"), value: MeasureTypes.TYPE_PREPEND_TO_SELECTION })
                        result.push({ text: qsTr("After selection"), value: MeasureTypes.TYPE_APPEND_TO_SELECTION })
                    }

                    return result
                }
            }
        }

        SeparatorLine { anchors.margins: -10 }

        FlatButton {
            icon:  IconCode.PLUS

            onClicked: {
                if (model) {
                    model.insertMeasures()
                }
            }
        }
    }
}
