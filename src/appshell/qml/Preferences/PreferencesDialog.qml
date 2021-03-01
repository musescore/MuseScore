import QtQuick 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

import "internal"

QmlDialog {
    id: root

    width: 880
    height: 600

    title: qsTrc("appshell", "Preferences")

    Rectangle {
        anchors.fill: parent

        color: ui.theme.backgroundSecondaryColor

        PreferencesModel {
            id: preferencesModel
        }

        SeparatorLine { id: topSeparator; anchors.top: parent.top }

        PreferencesMenu {
            id: menu

            anchors.top: topSeparator.bottom
            anchors.bottom: buttonsPanel.top
            anchors.left: parent.left

            SeparatorLine { orientation: Qt.Vertical; anchors.right: parent.right }
        }

        Item {
            anchors.top: topSeparator.bottom
            anchors.bottom: buttonsPanel.top
            anchors.left: menu.right
            anchors.right: parent.right
        }

        PreferencesButtonsPanel {
            id: buttonsPanel

            anchors.bottom: parent.bottom

            SeparatorLine { anchors.top: parent.top }

            onRevertFactorySettingsRequested: {
                preferencesModel.resetFactorySettings()
            }

            onRejectRequested: {
                root.reject()
            }

            onApplyRequested: {
                if (preferencesModel.apply()) {
                    root.hide()
                }
            }
        }
    }
}
