import QtQuick 2.9
import QtQuick.Layouts 1.3

import MuseScore.UiComponents 1.0
import MuseScore.Languages 1.0

Item {
    id: root

    property var language: undefined

    QtObject {
        id: privateProperties

        property string _currentOperation: "undefined" // "install" "update"

        function _currentOperationButton() {
            var currentButton
            if (privateProperties._currentOperation === "install") {
                currentButton = installButton
            } else if (privateProperties._currentOperation === "update") {
                currentButton = updateButton
            }

            return currentButton
        }
    }

    function setData(data) {
        language = data
    }

    function setProgress(status, indeterminate, current, total) {
        var currentButton = privateProperties._currentOperationButton()
        if (!currentButton) {
            return
        }

        currentButton.setProgress(status, indeterminate, current, total)
    }

    function resetProgress() {
        var currentButton = privateProperties._currentOperationButton()
        if (!currentButton) {
            return
        }

        currentButton.resetProgress()
    }

    signal install(string code)
    signal update(string code)
    signal uninstall(string code)
    signal openPreferences()

    Column {
        id: contentColumn

        anchors.fill: parent

        spacing: 42

        StyledTextLabel {
            width: 585

            font.pixelSize: 22
            font.bold: true
            horizontalAlignment: Text.AlignLeft

            text: Boolean(language) ? language.name : ""
        }

        RowLayout {
            anchors.left: parent.left
            anchors.right: parent.right

            spacing: 19

            FlatButton {
                Layout.alignment: Qt.AlignLeft
                text: qsTrc("languages", "Open language preferences")

                onClicked: {
                    privateProperties._currentOperation = "openPreferences"
                    root.openPreferences()
                }
            }

            Row {
                Layout.alignment: Qt.AlignRight

                spacing: 19

                ProgressButton {
                    id: updateButton

                    visible: Boolean(language) ? (language.status === LanguageStatus.NeedUpdate) : false

                    text: qsTrc("languages", "Update available")

                    onClicked: {
                        privateProperties._currentOperation = "update"
                        root.update(language.code)
                    }
                }
                ProgressButton {
                    id: installButton

                    visible: Boolean(language) ? (language.status === LanguageStatus.NoInstalled) : false

                    text: qsTrc("languages", "Install")

                    onClicked: {
                        privateProperties._currentOperation = "install"
                        root.install(language.code)
                    }
                }
                FlatButton {
                    visible: Boolean(language) ? (language.status === LanguageStatus.Installed ||
                                                  language.status === LanguageStatus.NeedUpdate) : false

                    text: qsTrc("languages", "Uninstall")

                    onClicked: {
                        privateProperties._currentOperation = ""
                        root.uninstall(language.code)
                    }
                }
            }
        }
    }
}
