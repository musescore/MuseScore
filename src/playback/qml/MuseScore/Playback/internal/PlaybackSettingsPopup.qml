import QtQuick 2.7

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Playback 1.0

StyledPopupView {
    id: root

    positionDisplacementX: parent.width/2 - width/2
    positionDisplacementY: parent.height

    arrowVisible: false

    Item {
        implicitWidth: 286
        implicitHeight: 188

        PlaybackSettingsModel {
            id: settingsModel
        }

        Component.onCompleted: {
            settingsModel.load()
        }

        ListView {
            id: view

            anchors.fill: parent
            anchors.topMargin: 4

            model: settingsModel

            interactive: false
            spacing: 0

            section.property: "section"
            section.delegate: SeparatorLine {
                //! NOTE: hide the first separator
                required property string section

                Component.onCompleted: {
                    height = section === "main" ? 0 : 1
                }
            }

            delegate: ListItemBlank {
                Row {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                    anchors.right: parent.right

                    spacing: 12

                    StyledIconLabel {
                        iconCode: model.checked ? IconCode.TICK_RIGHT_ANGLE : IconCode.NONE

                        width: 16
                        height: width
                    }

                    StyledIconLabel {
                        iconCode: model.icon
                    }

                    StyledTextLabel {
                        text: model.description
                    }
                }

                onClicked: {
                    settingsModel.handleAction(model.code)
                }
            }
        }
    }
}
