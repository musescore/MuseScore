import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspectors 3.3
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

        CheckBox {
            isIndeterminate: root.model ? root.model.isSmall.isUndefined : false
            checked: root.model && !isIndeterminate ? root.model.isSmall.value : false
            text: qsTr("Cue size")

            onClicked: { root.model.isSmall.value = !checked }
        }

        Item {
            height: childrenRect.height
            width: parent.width

            Column {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                spacing: 8

                StyledTextLabel {
                    text: qsTr("Offset")
                }

                IncrementalPropertyControl {
                    isIndeterminate: root.model ? root.model.verticalOffset.isUndefined : false
                    currentValue: root.model ? root.model.verticalOffset.value : 0
                    icon: IconCode.VERTICAL

                    step: 0.1
                    decimals: 2
                    minValue: 0.1
                    maxValue: 5

                    onValueEdited: { root.model.verticalOffset.value = newValue }
                }
            }

            Column {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                spacing: 8

                StyledTextLabel {
                    text: qsTr("Scale")
                }

                IncrementalPropertyControl {
                    id: scaleControl

                    isIndeterminate: root.model ? root.model.scale.isUndefined : false
                    currentValue: root.model ? root.model.scale.value : 0
                    iconMode: iconModeEnum.hidden

                    measureUnitsSymbol: "%"
                    step: 20
                    decimals: 2
                    maxValue: 400
                    minValue: 20

                    onValueEdited: { root.model.scale.value = newValue }
                }
            }
        }

        SeparatorLine { anchors.margins: -10 }

        Item {
            height: childrenRect.height
            width: parent.width

            Column {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                spacing: 8

                StyledTextLabel {
                    text: qsTr("Number of lines")
                }

                IncrementalPropertyControl {
                    id: lineCountControl

                    isIndeterminate: root.model ? root.model.lineCount.isUndefined : false
                    currentValue: root.model ? root.model.lineCount.value : 0
                    iconMode: iconModeEnum.hidden

                    step: 1
                    decimals: 0
                    maxValue: 14
                    minValue: 1
                    validator: IntInputValidator {
                        top: lineCountControl.maxValue
                        bottom: lineCountControl.minValue
                    }

                    onValueEdited: { root.model.lineCount.value = newValue }
                }
            }

            Column {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                spacing: 8

                StyledTextLabel {
                    text: qsTr("Line distance")
                }

                IncrementalPropertyControl {
                    id: lineDistanceControl

                    isIndeterminate: root.model ? root.model.lineDistance.isUndefined : false
                    currentValue: root.model ? root.model.lineDistance.value : 0
                    iconMode: iconModeEnum.hidden

                    step: 1
                    decimals: 0
                    maxValue: 3
                    minValue: 0
                    validator: IntInputValidator {
                        top: lineDistanceControl.maxValue
                        bottom: lineDistanceControl.minValue
                    }

                    onValueEdited: { root.model.lineDistance.value = newValue }
                }
            }
        }

        Column {
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            spacing: 8

            StyledTextLabel {
                text: qsTr("Step offset")
            }

            IncrementalPropertyControl {
                id: stepOffsetControl

                isIndeterminate: root.model ? root.model.stepOffset.isUndefined : false
                currentValue: root.model ? root.model.stepOffset.value : 0
                iconMode: iconModeEnum.hidden

                step: 1
                decimals: 0
                maxValue: 8
                minValue: -8
                validator: IntInputValidator {
                    top: stepOffsetControl.maxValue
                    bottom: stepOffsetControl.minValue
                }

                onValueEdited: { root.model.stepOffset.value = newValue }
            }
        }

        SeparatorLine { anchors.margins: -10 }

        Column {
            spacing: 8

            width: parent.width

            StyledTextLabel {
                text: qsTr("Notehead scheme")
            }

            StyledComboBox {
                width: parent.width

                textRoleName: "text"
                valueRoleName: "value"

                model: [
                    { text: qsTr("Auto"), value: NoteHead.SCHEME_AUTO },
                    { text: qsTr("Normal"), value: NoteHead.SCHEME_NORMAL },
                    { text: qsTr("Pitch names"), value: NoteHead.SCHEME_PITCHNAME },
                    { text: qsTr("German pitch names"), value: NoteHead.SCHEME_PITCHNAME_GERMAN },
                    { text: qsTr("Solfege movable Do"), value: NoteHead.SCHEME_SOLFEGE },
                    { text: qsTr("Solfege fixed Do"), value: NoteHead.SCHEME_SOLFEGE_FIXED },
                    { text: qsTr("4-shape (Walker)"), value: NoteHead.SCHEME_SHAPE_NOTE_4 },
                    { text: qsTr("7-shape (Aikin)"), value: NoteHead.SCHEME_SHAPE_NOTE_7_AIKIN },
                    { text: qsTr("7-shape (Funk)"), value: NoteHead.SCHEME_SHAPE_NOTE_7_FUNK },
                    { text: qsTr("7-shape (Walker)"), value: NoteHead.SCHEME_SHAPE_NOTE_7_WALKER }
                ]

                currentIndex: root.model && !root.model.noteheadSchemeType.isUndefined ? indexOfValue(root.model.noteheadSchemeType.value) : -1

                onValueChanged: {
                    root.model.noteheadSchemeType.value = value
                }
            }
        }

        Column {
            spacing: 6

            width: parent.width

            CheckBox {
                isIndeterminate: root.model ? root.model.isStemless.isUndefined : false
                checked: root.model && !isIndeterminate ? root.model.isStemless.value : false
                text: qsTr("Stemless")

                onClicked: { root.model.isStemless.value = !checked }
            }

            CheckBox {
                isIndeterminate: root.model ? root.model.shouldShowBarlines.isUndefined : false
                checked: root.model && !isIndeterminate ? root.model.shouldShowBarlines.value : false
                text: qsTr("Show barlines")

                onClicked: { root.model.shouldShowBarlines.value = !checked }
            }

            CheckBox {
                isIndeterminate: root.model ? root.model.shouldShowLedgerLines.isUndefined : false
                checked: root.model && !isIndeterminate ? root.model.shouldShowLedgerLines.value : false
                text: qsTr("Show ledger lines")

                onClicked: { root.model.shouldShowLedgerLines.value = !checked }
            }

            CheckBox {
                isIndeterminate: root.model ? root.model.shouldGenerateClefs.isUndefined : false
                checked: root.model && !isIndeterminate ? root.model.shouldGenerateClefs.value : false
                text: qsTr("Generate clefs")

                onClicked: { root.model.shouldGenerateClefs.value = !checked }
            }

            CheckBox {
                isIndeterminate: root.model ? root.model.shouldGenerateTimeSignatures.isUndefined : false
                checked: root.model && !isIndeterminate ? root.model.shouldGenerateTimeSignatures.value : false
                text: qsTr("Generate time signatures")

                onClicked: { root.model.shouldGenerateTimeSignatures.value = !checked }
            }

            CheckBox {
                isIndeterminate: root.model ? root.model.shouldGenerateKeySignatures.isUndefined : false
                checked: root.model && !isIndeterminate ? root.model.shouldGenerateKeySignatures.value : false
                text: qsTr("Generate key signatures")

                onClicked: { root.model.shouldGenerateKeySignatures.value = !checked }
            }
        }
    }
}
