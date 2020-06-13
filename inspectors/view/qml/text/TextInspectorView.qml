import QtQuick 2.9
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Inspectors 3.3
import "../common"

InspectorSectionView {
    id: root

    implicitHeight: contentColumn.height
    contentHeight: implicitHeight + textAdvancedSettingsButton.popupContentHeight

    Column {
        id: contentColumn

        height: implicitHeight
        width: parent.width

        spacing: 16

        Column {
            spacing: 8

            width: parent.width

            StyledTextLabel {
                text: qsTr("Font")
            }

            StyledComboBox {
                id: fontFamilyComboBox

                width: parent.width

                textRoleName: "text"
                valueRoleName: textRoleName

                model: {
                    var resultList = []

                    var fontFamilies = Qt.fontFamilies()

                    for (var i = 0; i < fontFamilies.length; ++i) {
                        resultList.push({"text" : fontFamilies[i]})
                    }

                    return resultList
                }

                currentIndex: root.model && !root.model.fontFamily.isUndefined ? indexOfValue(root.model.fontFamily.value) : -1

                onValueChanged: {
                    root.model.fontFamily.value = value
                }
            }
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
                    text: qsTr("Style")
                }

                RadioButtonGroup {
                    height: 30
                    width: implicitWidth

                    model: [
                        { iconRole: IconCode.TEXT_BOLD, valueRole: TextTypes.FONT_STYLE_BOLD },
                        { iconRole: IconCode.TEXT_ITALIC, valueRole: TextTypes.FONT_STYLE_ITALIC  },
                        { iconRole: IconCode.TEXT_UNDERLINE, valueRole: TextTypes.FONT_STYLE_UNDERLINE  }
                    ]

                    delegate: FlatToogleButton {

                        backgroundColor: "#00000000"
                        icon: modelData["iconRole"]

                        checked: root.model && !root.model.fontStyle.isUndefined ? root.model.fontStyle.value & modelData["valueRole"]
                                                                                 : false

                        onToggled: {
                            root.model.fontStyle.value = checked ? root.model.fontStyle.value & ~modelData["valueRole"]
                                                                 : root.model.fontStyle.value | modelData["valueRole"]
                        }
                    }
                }
            }

            Column {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                spacing: 8

                StyledTextLabel {
                    text: qsTr("Size")
                }

                StyledComboBox {
                    width: parent.width

                    textRoleName: "text"
                    valueRoleName: "value"

                    model: [
                        { text: "8",  value: 8 },
                        { text: "9",  value: 9 },
                        { text: "10", value: 10 },
                        { text: "11", value: 11 },
                        { text: "12", value: 12 },
                        { text: "14", value: 14 },
                        { text: "16", value: 16 },
                        { text: "18", value: 18 },
                        { text: "24", value: 24 },
                        { text: "30", value: 30 },
                        { text: "36", value: 36 },
                        { text: "48", value: 48 }
                    ]

                    currentIndex: root.model && !root.model.fontSize.isUndefined ? indexOfValue(root.model.fontSize.value) : -1

                    onValueChanged: {
                        root.model.fontSize.value = value
                    }
                }
            }
        }

        Column {
            spacing: 8

            width: parent.width

            StyledTextLabel {
                text: qsTr("Alignment")
            }

            Item {
                height: childrenRect.height
                width: parent.width

                RadioButtonGroup {
                    id: horizontalAlignmentButtonList

                    anchors.left: parent.left
                    anchors.right: parent.horizontalCenter
                    anchors.rightMargin: 2

                    height: 30

                    model: [
                        { iconRole: IconCode.TEXT_ALIGN_LEFT, typeRole: TextTypes.FONT_ALIGN_H_LEFT },
                        { iconRole: IconCode.TEXT_ALIGN_CENTER, typeRole: TextTypes.FONT_ALIGN_H_CENTER },
                        { iconRole: IconCode.TEXT_ALIGN_RIGHT, typeRole: TextTypes.FONT_ALIGN_H_RIGHT }
                    ]

                    delegate: FlatRadioButton {

                        width: 30

                        backgroundColor: "#00000000"

                        ButtonGroup.group: horizontalAlignmentButtonList.radioButtonGroup

                        checked: root.model && !root.model.horizontalAlignment.isUndefined ? root.model.horizontalAlignment.value === modelData["typeRole"]
                                                                                           : false

                        onToggled: {
                            root.model.horizontalAlignment.value = modelData["typeRole"]
                        }

                        StyledIconLabel {
                            iconCode: modelData["iconRole"]
                        }
                    }
                }

                RadioButtonGroup {
                    id: verticalAlignmentButtonList

                    anchors.left: parent.horizontalCenter
                    anchors.leftMargin: 2
                    anchors.right: parent.right

                    height: 30

                    model: [
                        { iconRole: IconCode.TEXT_ALIGN_UNDER, typeRole: TextTypes.FONT_ALIGN_V_BOTTOM },
                        { iconRole: IconCode.TEXT_ALIGN_MIDDLE, typeRole: TextTypes.FONT_ALIGN_V_CENTER },
                        { iconRole: IconCode.TEXT_ALIGN_BASELINE, typeRole: TextTypes.FONT_ALIGN_V_BASELINE },
                        { iconRole: IconCode.TEXT_ALIGN_ABOVE, typeRole: TextTypes.FONT_ALIGN_V_TOP }
                    ]

                    delegate: FlatRadioButton {

                        width: 30

                        backgroundColor: "#00000000"

                        ButtonGroup.group: verticalAlignmentButtonList.radioButtonGroup

                        checked: root.model && !root.model.verticalAlignment.isUndefined ? root.model.verticalAlignment.value === modelData["typeRole"]
                                                                                         : false

                        onToggled: {
                            root.model.verticalAlignment.value = modelData["typeRole"]
                        }

                        StyledIconLabel {
                            iconCode: modelData["iconRole"]
                        }
                    }
                }
            }
        }

        FlatButton {
            text: qsTr("Insert special charachters")

            visible: root.model ? root.model.isSpecialCharactersInsertionAvailable : false

            onClicked: {
                if (root.model) {
                    root.model.insertSpecialCharacters()
                }
            }
        }

        TextAdvancedSettings {
            id: textAdvancedSettingsButton

            width: contentColumn.width
            popupAvailableWidth: contentColumn.width
            popupPositionX: mapToGlobal(contentColumn.x, contentColumn.y).x - mapToGlobal(x, y).x

            model: root.model
        }
    }
}
