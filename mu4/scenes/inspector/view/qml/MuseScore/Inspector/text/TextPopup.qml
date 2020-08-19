import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Inspector 1.0
import "../common"

StyledPopup {
    id: root

    property QtObject model: null

    width: parent.width

    contentItem: Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        Item {
            height: childrenRect.height
            width: parent.width

            CheckBox {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2
                anchors.verticalCenter: subscriptOptionsButtonList.verticalCenter

                isIndeterminate: model ? model.isSizeSpatiumDependent.isUndefined : false
                checked: model && !isIndeterminate ? model.isSizeSpatiumDependent.value : false
                text: qsTr("Match staff size")

                onClicked: { model.isSizeSpatiumDependent.value = !checked }
            }

            RadioButtonGroup {
                id: subscriptOptionsButtonList

                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                height: 30

                layoutDirection: Qt.RightToLeft

                model: [
                    { iconRole: IconCode.TEXT_SUPERSCRIPT, typeRole: TextTypes.TEXT_SUBSCRIPT_TOP },
                    { iconRole: IconCode.TEXT_SUBSCRIPT, typeRole: TextTypes.TEXT_SUBSCRIPT_BOTTOM }
                ]

                delegate: FlatRadioButton {

                    width: 30

                    backgroundColor: "#00000000"

                    ButtonGroup.group: subscriptOptionsButtonList.radioButtonGroup

                    checked: root.model && !root.model.textScriptAlignment.isUndefined ? root.model.textScriptAlignment.value === modelData["typeRole"]
                                                                                       : false

                    onClicked: {
                        if (root.model.textScriptAlignment.value === modelData["typeRole"]) {
                            root.model.textScriptAlignment.value = TextTypes.TEXT_SUBSCRIPT_NORMAL
                        } else {
                            root.model.textScriptAlignment.value = modelData["typeRole"]
                        }
                    }

                    StyledIconLabel {
                        iconCode: modelData["iconRole"]
                    }
                }
            }
        }

        InspectorPropertyView {
            titleText: qsTr("Frame")
            propertyItem: root.model ? root.model.frameType : null

            RadioButtonGroup {
                id: frameType

                height: 30
                width: parent.width

                model: [
                    { iconRole: IconCode.NONE, typeRole: TextTypes.FRAME_TYPE_NONE },
                    { iconRole: IconCode.FRAME_SQUARE, typeRole: TextTypes.FRAME_TYPE_SQUARE },
                    { iconRole: IconCode.FRAME_CIRCLE, typeRole: TextTypes.FRAME_TYPE_CIRCLE }
                ]

                delegate: FlatRadioButton {

                    ButtonGroup.group: frameType.radioButtonGroup

                    checked: root.model && !root.model.frameType.isUndefined ? root.model.frameType.value === modelData["typeRole"]
                                                                             : false

                    onToggled: {
                        root.model.frameType.value = modelData["typeRole"]
                    }

                    StyledIconLabel {
                        iconCode: modelData["iconRole"]
                    }
                }
            }
        }

        Item {
            height: childrenRect.height
            width: parent.width

            InspectorPropertyView {
                id: frameBorderColorColumn

                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                visible: root.model ? root.model.frameBorderColor.isEnabled : false
                height: visible ? implicitHeight : 0

                titleText: qsTr("Border")
                propertyItem: root.model ? root.model.frameBorderColor : null

                ColorPicker {
                    isIndeterminate: root.model  ? root.model.frameBorderColor.isUndefined : false
                    color: root.model && !root.model.frameBorderColor.isUndefined ? root.model.frameBorderColor.value : ui.theme.backgroundPrimaryColor

                    onNewColorSelected: {
                        if (root.model) {
                            root.model.frameBorderColor.value = newColor
                        }
                    }
                }
            }

            InspectorPropertyView {
                id: frameHighlightColorColumn

                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                visible: root.model ? root.model.frameHighlightColor.isEnabled : false
                height: visible ? implicitHeight : 0

                titleText: qsTr("Highlight")
                propertyItem: root.model ? root.model.frameHighlightColor : null

                ColorPicker {
                    isIndeterminate: root.model ? root.model.frameHighlightColor.isUndefined : false
                    color: root.model && !root.model.frameHighlightColor.isUndefined ? root.model.frameHighlightColor.value : ui.theme.backgroundPrimaryColor

                    onNewColorSelected: {
                        if (root.model) {
                            root.model.frameHighlightColor.value = newColor
                        }
                    }
                }
            }
        }

        Item {
            height: childrenRect.height
            width: parent.width

            InspectorPropertyView {
                id: frameThicknessColumn

                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                visible: root.model ? root.model.frameThickness.isEnabled : false
                height: visible ? implicitHeight : 0

                titleText: qsTr("Thickness")
                propertyItem: root.model ? root.model.frameThickness : null

                IncrementalPropertyControl {
                    iconMode: iconModeEnum.hidden

                    isIndeterminate: root.model ? root.model.frameThickness.isUndefined : false
                    currentValue: root.model ? root.model.frameThickness.value : 0

                    step: 0.1
                    minValue: 0.1
                    maxValue: 5

                    onValueEdited: { root.model.frameThickness.value = newValue }
                }
            }

            InspectorPropertyView {
                id: frameMarginColumn

                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                visible: root.model ? root.model.frameMargin.isEnabled : false
                height: visible ? implicitHeight : 0

                titleText: qsTr("Margin")
                propertyItem: root.model ? root.model.frameMargin : null

                IncrementalPropertyControl {
                    iconMode: iconModeEnum.hidden

                    isIndeterminate: root.model ? root.model.frameMargin.isUndefined : false
                    currentValue: root.model ? root.model.frameMargin.value : 0

                    step: 0.1
                    minValue: 0
                    maxValue: 5

                    onValueEdited: { root.model.frameMargin.value = newValue }
                }
            }
        }

        InspectorPropertyView {
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            visible: root.model ? root.model.frameCornerRadius.isEnabled : false
            height: visible ? implicitHeight : 0

            titleText: qsTr("Corner radius")
            propertyItem: root.model ? root.model.frameCornerRadius : null

            IncrementalPropertyControl {
                iconMode: iconModeEnum.hidden

                isIndeterminate: root.model ? root.model.frameCornerRadius.isUndefined : false
                currentValue: root.model ? root.model.frameCornerRadius.value : 0

                step: 0.1
                minValue: 0
                maxValue: 5

                onValueEdited: { root.model.frameCornerRadius.value = newValue }
            }
        }

        SeparatorLine { anchors.margins: -10 }

        InspectorPropertyView {
            titleText: qsTr("Text style")
            propertyItem: root.model ? root.model.textType : null

            StyledComboBox {
                width: parent.width

                textRoleName: "text"
                valueRoleName: "value"

                model: [
                    { text: qsTr("Title"), value: TextTypes.TEXT_TYPE_TITLE },
                    { text: qsTr("Subtitle"), value: TextTypes.TEXT_TYPE_SUBTITLE},
                    { text: qsTr("Composer"), value: TextTypes.TEXT_TYPE_COMPOSER },
                    { text: qsTr("Lyricist"), value: TextTypes.TEXT_TYPE_LYRICS_ODD },
                    { text: qsTr("Translator"), value: TextTypes.TEXT_TYPE_TRANSLATOR },
                    { text: qsTr("Frame"), value: TextTypes.TEXT_TYPE_FRAME },
                    { text: qsTr("Header"), value: TextTypes.TEXT_TYPE_HEADER },
                    { text: qsTr("Footer"), value: TextTypes.TEXT_TYPE_FOOTER },
                    { text: qsTr("Measure number"), value: TextTypes.TEXT_TYPE_MEASURE_NUMBER },
                    { text: qsTr("Instrument name (Part)"), value: TextTypes.TEXT_TYPE_INSTRUMENT_EXCERPT },
                    { text: qsTr("Instrument change"), value: TextTypes.TEXT_TYPE_INSTRUMENT_CHANGE },
                    { text: qsTr("Staff"), value: TextTypes.TEXT_TYPE_STAFF },
                    { text: qsTr("System"), value: TextTypes.TEXT_TYPE_SYSTEM },
                    { text: qsTr("Expression"), value: TextTypes.TEXT_TYPE_EXPRESSION },
                    { text: qsTr("Dynamics"), value: TextTypes.TEXT_TYPE_DYNAMICS },
                    { text: qsTr("Hairpin"), value: TextTypes.TEXT_TYPE_HAIRPIN },
                    { text: qsTr("Tempo"), value: TextTypes.TEXT_TYPE_TEMPO },
                    { text: qsTr("Rehearshal mark"), value: TextTypes.TEXT_TYPE_REHEARSAL_MARK },
                    { text: qsTr("Repeat text left"), value: TextTypes.TEXT_TYPE_REPEAT_LEFT },
                    { text: qsTr("Repeat text right"), value: TextTypes.TEXT_TYPE_REPEAT_RIGHT },
                    { text: qsTr("Lyrics odd lines"), value: TextTypes.TEXT_TYPE_LYRICS_ODD },
                    { text: qsTr("Lyrics even lines"), value: TextTypes.TEXT_TYPE_LYRICS_EVEN },
                    { text: qsTr("Chord symbol"), value: TextTypes.TEXT_TYPE_HARMONY_A },
                    { text: qsTr("Chord symbol (Alternate)"), value: TextTypes.TEXT_TYPE_HARMONY_B },
                    { text: qsTr("Roman numeral analysis"), value: TextTypes.TEXT_TYPE_HARMONY_ROMAN },
                    { text: qsTr("Nashville number"), value: TextTypes.TEXT_TYPE_HARMONY_NASHVILLE },
                    { text: qsTr("Sticking"), value: TextTypes.TEXT_TYPE_STICKING }
                ]

                currentIndex: root.model && !root.model.textType.isUndefined ? indexOfValue(root.model.textType.value) : -1

                onValueChanged: {
                    root.model.textType.value = value
                }
            }
        }

        RadioButtonGroup {
            id: textPositionButtonList

            height: 30
            width: parent.width

            model: [
                { textRole: qsTr("Above"), valueRole: TextTypes.TEXT_PLACEMENT_ABOVE },
                { textRole: qsTr("Below"), valueRole: TextTypes.TEXT_PLACEMENT_BELOW }
            ]

            delegate: FlatRadioButton {

                ButtonGroup.group: textPositionButtonList.radioButtonGroup

                checked: root.model && !root.model.textPlacement.isUndefined ? root.model.textPlacement.value === modelData["valueRole"]
                                                                             : false
                onToggled: {
                    root.model.textPlacement.value = modelData["valueRole"]
                }

                StyledTextLabel {
                    text: modelData["textRole"]

                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        FlatButton {
            width: parent.width

            text: qsTr("Staff text properties")

            visible: root.model ? root.model.areStaffTextPropertiesAvailable : false

            onClicked: {
                root.model.showStaffTextProperties()
            }
        }
    }
}
