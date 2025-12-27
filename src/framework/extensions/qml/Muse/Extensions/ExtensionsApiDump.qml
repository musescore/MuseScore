import QtQuick

import Muse.Ui
import Muse.UiComponents
import Muse.Extensions

Rectangle {
    id: root

    objectName: "DiagnosticExtensionsApiDump"
    color: ui.theme.backgroundPrimaryColor

    Component.onCompleted: {
        apiModel.load();
    }

    Item {
        id: toolPanel
        anchors.left: parent.left
        anchors.right: parent.right
        height: 48

        StyledDropdown {
            id: types

            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            width: 200

            currentIndex: 1 // 1 - extensions
            model: apiModel.apiTypes()

            onActivated: function(index, value) {
                currentIndex = index
                apiModel.setApiType(value)
            }
        }

        TextInputField {
            id: inputItem
            anchors.left: types.right
            anchors.right: btnRow.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: 16
            clearTextButtonVisible: true
            onTextChanged: function(newTextValue) {
                apiModel.find(newTextValue)
            }
        }

        Row {
            id: btnRow
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.rightMargin: 16
            width: childrenRect.width
            spacing: 8

            FlatButton {
                anchors.verticalCenter: parent.verticalCenter
                text: "Copy wiki"
                onClicked: apiModel.copyWiki()
            }

            FlatButton {
                anchors.verticalCenter: parent.verticalCenter
                text: "Print wiki"
                onClicked: apiModel.printWiki()
            }
        }
    }

    ApiDumpModel {
        id: apiModel
    }

    ListView {
        anchors.top: toolPanel.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        clip: true
        model: apiModel

        section.property: "groupRole"
        section.delegate: Rectangle {
            id: sectionDelegate
            required property string section

            width: parent.width
            height: 24
            color: ui.theme.backgroundSecondaryColor

            StyledTextLabel {
                anchors.fill: parent
                anchors.margins: 2
                horizontalAlignment: Qt.AlignLeft
                text: sectionDelegate.section
            }
        }

        delegate: ListItemBlank {
            id: itemDelegate

            required property string data

            anchors.left: parent ? parent.left : undefined
            anchors.right: parent ? parent.right : undefined
            height: 24

            StyledTextLabel {
                anchors.fill: parent
                anchors.leftMargin: 16
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                font.family: "Consolas"
                text: itemDelegate.data
            }
        }
    }
}
