import QtQuick 2.12

import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Rectangle {
    id: root

    visible: false
    height: 50

    color: ui.theme.backgroundPrimaryColor

    QtObject {
        id: privateProperties

        function show() {
            visible = true
            Qt.callLater(textInputField.forceActiveFocus)
        }

        function hide() {
            visible = false
        }
    }

    SearchPopupModel {
        id: model

        onShowPopupRequested: {
            privateProperties.show()
        }
    }

    Component.onCompleted: {
        model.load()
    }

    Row {
        anchors.verticalCenter: parent.verticalCenter

        spacing: 8

        FlatButton {
            id: closeButton

            icon: IconCode.CLOSE_X_ROUNDED

            onClicked: {
                privateProperties.hide()
            }
        }

        TextInputField {
            id: textInputField

            Component.onCompleted: {
                forceActiveFocus()
            }

            width: 500

            onCurrentTextEdited: {
                model.search(newTextValue)
            }
        }
    }
}
