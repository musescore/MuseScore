import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

ListItemBlank {
    id: root

    property var item: null
    property string keyRoleName: "key"
    property string valueRoleName: "value"

    property bool readOnly: false

    property alias spacing: row.spacing
    property real sideMargin: 0
    property real valueItemWidth: 126

    width: ListView.view.width
    height: 34

    normalStateColor: (index % 2 == 0) ? ui.theme.backgroundSecondaryColor
                                       : ui.theme.backgroundPrimaryColor

    QtObject {
        id: privateProperties

        function componentByType(type) {
            switch (type) {
            case "Undefined": return textComp
            case "Bool": return boolComp
            case "Int": return numberComp
            case "Double": return numberComp
            case "String": return textComp
            case "Color": return colorComp
            }

            return textComp
        }

        function isNumberComponent() {
            return item.valueType === "Int" || type.valueType === "Double"
        }
    }

    onClicked: {
        forceActiveFocus()
    }

    RowLayout {
        id: row

        anchors.fill: parent

        StyledTextLabel {
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.fillWidth: true
            Layout.leftMargin: root.sideMargin

            text: root.item[keyRoleName]
            horizontalAlignment: Text.AlignLeft
        }

        Loader {
            id: loader
            property var val: root.item[valueRoleName]

            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.preferredWidth: root.valueItemWidth
            Layout.rightMargin: root.sideMargin

            sourceComponent: !root.readOnly ? privateProperties.componentByType(root.item.valueType) : readOnlyComponent

            onLoaded: {
                loader.item.val = loader.val

                if (privateProperties.isNumberComponent() && !root.readOnly) {
                    if (Boolean(root.item.min)) {
                        loader.item.minValue = root.item.min
                    }

                    if (Boolean(root.item.max)) {
                        loader.item.maxValue = root.item.max
                    }
                }
            }

            onValChanged: {
                if (loader.item) {
                    loader.item.val = loader.val
                }
            }

            Connections {
                target: loader.item
                function onChanged(newVal) {
                    root.item[valueRoleName] = newVal
                }
            }
        }
    }

    Component {
        id: textComp
        TextInputField {
            id: textControl

            property string val
            signal changed(string newVal)

            currentText: val

            onCurrentTextEdited: {
                textControl.changed(newTextValue)
            }
        }
    }

    Component {
        id: colorComp
        ColorPicker {
            id: colorControl

            property color val
            signal changed(color newVal)

            color: val

            onNewColorSelected: {
                colorControl.changed(newColor)
            }
        }
    }

    Component {
        id: numberComp
        IncrementalPropertyControl {
            id: numberControl

            property int val
            signal changed(int newVal)

            iconMode: iconModeEnum.hidden

            currentValue: val

            step: 1

            onValueEdited: {
                numberControl.changed(Number(newValue))
            }
        }
    }

    Component {
        id: boolComp
        CheckBox {
            id: boolControl

            property bool val
            signal changed(bool newVal)

            checked: val ? true : false
            onClicked: {
                boolControl.changed(!boolControl.checked)
            }
        }
    }

    Component {
        id: readOnlyComponent

        StyledTextLabel {
            property var val
            signal changed(var stub)

            text: val
            horizontalAlignment: Text.AlignLeft
        }
    }
}
