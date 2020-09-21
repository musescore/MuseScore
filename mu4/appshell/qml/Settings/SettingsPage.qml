import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.2
import MuseScore.Ui 1.0
import MuseScore.Dock 1.0
import MuseScore.Settings 1.0


DockPage {
    id: settingsPage
    objectName: "settings"

    central: DockCentral {
        id: settingsCentral
        objectName: "settingsCentral"

        Rectangle {

            id: content

            SettingListModel {
                id: settingsModel
            }

            Component.onCompleted: {
                settingsModel.load()
            }

            ListView {
                anchors.fill: parent

                model: settingsModel

                section.property: "sectionRole"
                section.delegate: Rectangle {
                    width: parent.width
                    height: 24
                    color: "#cccccc"
                    Text {
                        anchors.fill: parent
                        anchors.margins: 2
                        verticalAlignment: Text.AlignVCenter
                        text: section
                    }
                }

                delegate: Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    height: 32
                    color: content.color

                    Text {
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.right: control.left
                        verticalAlignment: Text.AlignVCenter
                        text: keyRole
                    }

                    Item {
                        id: control
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.right: parent.right
                        anchors.margins: 2
                        width: 150

                        Loader {
                            id: loader
                            property var val: valRole
                            anchors.fill: parent
                            sourceComponent: content.componentByType(typeRole)
                            onLoaded: loader.item.val = loader.val
                            onValChanged: {
                                if (loader.item) {
                                    loader.item.val = loader.val
                                }
                            }
                        }

                        Connections {
                            target: loader.item
                            onChanged: settingsModel.changeVal(model.index, newVal)
                        }
                    }
                }
            }

            function componentByType(type) {
                switch (type) {
                case "Undefined": return textComp;
                case "Bool": return boolComp;
                case "Int": return intComp;
                case "Double": return doubleComp;
                case "String": return textComp;
                case "Color": return colorComp;
                }

                return textComp;
            }

            Component {
                id: textComp

                Rectangle {
                    id: textControl
                    property var val
                    signal changed(var newVal)
                    anchors.fill: parent
                    border.width: 1
                    border.color: "#333333"
                    TextEdit {
                        anchors.fill: parent
                        anchors.margins: 2
                        verticalAlignment: Text.AlignVCenter
                        text: val
                        onEditingFinished: textControl.changed(text)
                    }
                }
            }

            Component {
                id: colorComp
                Rectangle {
                    id: colorControl
                    property var val
                    signal changed(var newVal)
                    anchors.fill: parent
                    color: val

                    ColorDialog {
                        id: colorDialog
                        title: "Please choose a color"
                        onAccepted: colorControl.changed(colorDialog.color)
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: colorDialog.open()
                    }
                }
            }

            Component {
                id: intComp
                SpinBox {
                    id: spinbox
                    property var val
                    signal changed(var newVal)
                    anchors.fill: parent
                    value: val
                    stepSize: 1
                    textFromValue: function(value, locale) { return String(value) }
                    valueFromText: function(text, locale) { return Number(text) }
                    onValueModified: spinbox.changed(Number(spinbox.value))
                }
            }

            Component {
                id: doubleComp
                SpinBox {
                    id: spinbox
                    property var val
                    signal changed(var newVal)
                    anchors.centerIn: parent
                    value: val * 10
                    stepSize: 10
                    from: -1000
                    to: 1000

                    property int decimals: 1
                    property real realValue: value / 10

                    textFromValue: function(value, locale) {
                        return Number(value / 10).toLocaleString(locale, 'f', spinbox.decimals)
                    }

                    valueFromText: function(text, locale) {
                        return Number.fromLocaleString(locale, text) * 10
                    }
                }
            }

            Component {
                id: boolComp
                CheckBox {
                    id: checkbox
                    property var val
                    signal changed(var newVal)
                    anchors.fill: parent
                    checked: val ? true : false
                    onClicked: checkbox.changed(checkbox.checked ? true : false)
                }
            }
        }
    }
}
