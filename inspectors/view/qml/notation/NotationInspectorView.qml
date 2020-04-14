import QtQuick 2.0

import "../common"

FocusableItem {
    id: root

    property alias model: settingsPopup.proxyModel

    implicitHeight: noteSettingsButton.height

    anchors.left: parent.left
    anchors.right: parent.right
    anchors.rightMargin: 48

    FlatButton {
        id: noteSettingsButton

        icon: "qrc:/resources/icons/note.svg"
        iconPixelSize: 16
        text: qsTr("Note settings")

        onClicked: {
            if (settingsPopup.isOpened) {
                settingsPopup.close()
            } else {
                settingsPopup.open()
            }
        }
    }

    NoteSettingsPopup {
        id: settingsPopup

        x: noteSettingsButton.x
        y: noteSettingsButton.y + noteSettingsButton.height
    }
}
