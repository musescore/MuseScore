import QtQuick 2.9
import QtQuick.Layouts 1.3

import MuseScore.UiComponents 1.0

PopupPanel {
    id: root

    property string title
    property string author
    property string maintainer
    property string description

    property bool installed: false
    property bool hasUpdate: false

    signal installRequested()
    signal updateRequested()
    signal uninstallRequested()
    signal restartRequested()
    signal openFullDescriptionRequested()

    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom

    height: 360

    visible: false

    QtObject {
        id: privateProperties

        property var currentOperationButton: undefined
    }

    function setProgress(status, indeterminate, current, total) {
        if (!Boolean(privateProperties.currentOperationButton)) {
            return
        }

        privateProperties.currentOperationButton.setProgress(status, indeterminate, current, total)
    }

    function resetProgress() {
        if (!Boolean(privateProperties.currentOperationButton)) {
            return
        }

        privateProperties.currentOperationButton.resetProgress()
    }

    content: Column {
        anchors.fill: parent
        anchors.topMargin: 44
        anchors.leftMargin: 68
        anchors.rightMargin: 68
        anchors.bottomMargin: 42

        spacing: 42

        Column {
            width: 585

            spacing: 8

            StyledTextLabel {
                font.pixelSize: 22
                font.bold: true

                text: Boolean(root.title) ? root.title : ""
            }

            Row {
                anchors.left: parent.left
                anchors.right: parent.right

                spacing: 4

                StyledTextLabel {
                    font.pixelSize: 14
                    text: qsTrc("uicomponents", "Author:")
                }

                StyledTextLabel {
                    font.pixelSize: 14
                    font.bold: true
                    text: Boolean(root.author) ? root.author : qsTrc("uicomponents", "MuseScore")
                }

                Rectangle {
                    width: 2
                    height: parent.height - 4
                    anchors.verticalCenter: parent.verticalCenter
                    color: ui.theme.fontPrimaryColor
                }

                StyledTextLabel {
                    font.pixelSize: 14
                    text: qsTrc("uicomponents", "Maintained by:")
                }

                StyledTextLabel {
                    font.pixelSize: 14
                    font.bold: true
                    text: Boolean(root.maintainer) ? root.maintainer : qsTrc("uicomponents", "MuseScore")
                }
            }
        }

        StyledTextLabel {
            width: 585
            height: 88

            font.pixelSize: 12
            opacity: 0.75
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignTop
            horizontalAlignment: Text.AlignLeft

            text: Boolean(root.description) ? root.description : ""
        }

        RowLayout {
            anchors.left: parent.left
            anchors.right: parent.right

            spacing: 19

            FlatButton {
                id: openFullDescriptionButton
                Layout.alignment: Qt.AlignLeft
                text: qsTrc("uicomponents", "View full description")

                onClicked: {
                    privateProperties.currentOperationButton = undefined
                    root.openFullDescriptionRequested()
                }
            }

            Row {
                Layout.alignment: Qt.AlignRight

                spacing: 19

                ProgressButton {
                    id: updateButton

                    visible: root.hasUpdate

                    text: qsTrc("uicomponents", "Update available")

                    onClicked: {
                        privateProperties.currentOperationButton = updateButton
                        root.updateRequested()
                    }
                }

                FlatButton {
                    visible: root.installed || root.hasUpdate

                    text: qsTrc("uicomponents", "Restart")

                    onClicked: {
                        privateProperties.currentOperationButton = undefined
                        root.restartRequested()
                    }
                }

                ProgressButton {
                    id: installButton

                    visible: !root.installed

                    text: qsTrc("uicomponents", "Install")

                    onClicked: {
                        privateProperties.currentOperationButton = installButton
                        root.installRequested()
                    }
                }

                FlatButton {
                    id: uninstallButton

                    visible: root.installed

                    text: qsTrc("uicomponents", "Uninstall")

                    onClicked: {
                        privateProperties.currentOperationButton = undefined
                        root.uninstallRequested()
                    }
                }
            }
        }
    }
}
