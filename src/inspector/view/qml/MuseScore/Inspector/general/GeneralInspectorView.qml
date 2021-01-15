import QtQuick 2.9
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Inspector 1.0
import "../common"
import "playback"
import "appearance"

InspectorSectionView {
    id: root

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

                text: qsTrc("inspector", "Visible")

                isIndeterminate: model ? model.isVisible.isUndefined : false
                checked: model && !model.isVisible.isUndefined ? model.isVisible.value : false

                onClicked: { model.isVisible.value = !checked }
            }

            CheckBox {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 6

                text: qsTrc("inspector", "Cue size")

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

                text: qsTrc("inspector", "Auto-place")

                isIndeterminate: model ? model.isAutoPlaceAllowed.isUndefined : false
                checked: model && !model.isAutoPlaceAllowed.isUndefined ? model.isAutoPlaceAllowed.value : false

                onClicked: { model.isAutoPlaceAllowed.value = !checked }
            }

            CheckBox {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 6

                text: qsTrc("inspector", "Play")

                enabled: model ? model.isPlayable.isEnabled : false
                isIndeterminate: model && enabled ? model.isPlayable.isUndefined : false
                checked: model && !model.isPlayable.isUndefined && enabled ? model.isPlayable.value : false

                onClicked: { model.isPlayable.value = !checked }
            }
        }

        Row {
            id: popupButtonsRow

            width: parent.width

            spacing: 4

            FlatButton {
                id: playbackButton

                width: (parent.width - popupButtonsRow.spacing)/ 2

                icon: IconCode.AUDIO
                text: qsTrc("inspector", "Playback")

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

                    arrowX: (width - playbackButton.width - popupButtonsRow.spacing) / 2
                    x: popupButtonsRow.x
                    y: playbackButton.y + playbackButton.height
                }
            }

            FlatButton {
                id: appearanceButton

                width: (parent.width - popupButtonsRow.spacing)/ 2

                icon: IconCode.POSITION_ARROWS
                text: qsTrc("inspector", "Appearance")

                onClicked: {
                    if (appearancePopup.isOpened) {
                        appearancePopup.close()
                    } else {
                        appearancePopup.open()
                    }
                }

                AppearancePopup {
                    id: appearancePopup

                    model: root.model ? root.model.appearanceSettingsModel : null

                    width: popupButtonsRow.width

                    arrowX: (width + appearanceButton.width + popupButtonsRow.spacing) / 2
                    x: appearanceButton.x - popupButtonsRow.width - popupButtonsRow.spacing
                    y: appearanceButton.y + appearanceButton.height
                }
            }
        }
    }
}
