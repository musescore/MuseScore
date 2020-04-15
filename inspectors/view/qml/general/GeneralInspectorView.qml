import QtQuick 2.9
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.0
import "../common"
import "playback"

FocusableItem {
    id: root

    property QtObject model: undefined

    anchors.left: parent.left
    anchors.right: parent.right
    anchors.rightMargin: 48

    implicitHeight: contentColumn.height

    Column {
        id: contentColumn

        height: implicitHeight
        width: parent.width

        spacing: 16

        Item {

            height: childrenRect.height
            width: root.width

            CheckBox {
                anchors.left: parent.left

                text: qsTr("Visible")

                isIndeterminate: model ? model.isVisible.isUndefined : false
                checked: model && !model.isVisible.isUndefined ? model.isVisible.value : false

                onClicked: { model.isVisible.value = !checked }
            }

            CheckBox {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 6

                text: qsTr("Cue size")

                enabled: model ? model.isSmall.isEnabled : false
                isIndeterminate: model && enabled ? model.isSmall.isUndefined : false
                checked: model && !model.isSmall.isUndefined ? model.isSmall.value : false

                onClicked: { model.isSmall.value = !checked }
            }
        }

        Item {

            height: childrenRect.height
            width: root.width

            CheckBox {
                anchors.left: parent.left

                text: qsTr("Auto-place")

                isIndeterminate: model ? model.isAutoPlaceAllowed.isUndefined : false
                checked: model && !model.isAutoPlaceAllowed.isUndefined ? model.isAutoPlaceAllowed.value : false

                onClicked: { model.isAutoPlaceAllowed.value = !checked }
            }

            CheckBox {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 6

                text: qsTr("Play")

                enabled: model ? model.isPlayable.isEnabled : false
                isIndeterminate: model && enabled ? model.isPlayable.isUndefined : false
                checked: model && !model.isPlayable.isUndefined && enabled ? model.isPlayable.value : false

                onClicked: { model.isPlayable.value = !checked }
            }
        }

        Row {
            id: popupButtonsRow

            width: parent.width

            spacing: 12

            FlatButton {
                id: playbackButton

                width: (parent.width - popupButtonsRow.spacing)/ 2

                icon: "qrc:/resources/icons/playback.svg"
                iconPixelSize: 16
                text: qsTr("Playback")

                onClicked: {
                    if (playbackPopup.isOpened) {
                        playbackPopup.close()
                    } else {
                        playbackPopup.open()
                    }
                }

                PlaybackPopup {
                    id: playbackPopup

                    proxyModel: model ? model.playbackProxyModel : null

                    width: popupButtonsRow.width

                    x: playbackButton.x
                    y: playbackButton.y + playbackButton.height
                }
            }

            FlatButton {
                id: appearanceButton

                width: (parent.width - popupButtonsRow.spacing)/ 2

                icon: "qrc:/resources/icons/appearance.svg"
                iconPixelSize: 16
                text: qsTr("Appearance")
            }
        }
    }
}
