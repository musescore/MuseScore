import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.12

import MuseScore.Playback 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.NotationScene 1.0

Rectangle {
    id: root

    property bool floating: false

    PlaybackToolBarModel {
        id: playbackModel
    }

    Component.onCompleted: {
        playbackModel.load()
    }

    Column {
        spacing: 14

        anchors.verticalCenter: parent.verticalCenter

        width: parent.width

        RowLayout {
            id: playbackActions

            spacing: 2

            ListView {
                Layout.preferredWidth: childrenRect.width
                Layout.preferredHeight: childrenRect.height

                contentHeight: 32
                contentWidth: contentHeight

                model: FilterProxyModel {
                    sourceModel: playbackModel

                    filters: [
                        FilterValue {
                            roleName: "isAdditional"
                            roleValue: false
                            compareType: CompareType.Equal
                            enabled: !root.floating
                        }
                    ]
                }

                orientation: Qt.Horizontal
                interactive: false

                delegate: FlatButton {
                    id: playbackButton

                    icon: model.icon
                    hint: model.hint
                    enabled: model.enabled

                    normalStateColor: model.checked ? ui.theme.accentColor : "transparent"

                    onClicked: {
                        playbackModel.handleAction(model.code)
                    }
                }
            }

            SeparatorLine { orientation: Qt.Vertical }

            TimeInputField {
                id: timeField

                Layout.leftMargin: 20
                Layout.preferredWidth: 90

                time: playbackModel.playTime

                maxHours: 9
                maxMilliseconds: 9

                onTimeEdited: {
                    playbackModel.playTime = newTime
                }
            }

            NumberInputField {
                Layout.leftMargin: 20

                maxValue: playbackModel.maxBeatNumber
                value: playbackModel.measureNumber

                onValueChanged: {
                    playbackModel.measureNumber = value
                }
            }

            StyledTextLabel {
                text: "."
                font: timeField.font
            }

            NumberInputField {
                Layout.leftMargin: 4

                maxValue: playbackModel.maxBeatNumber
                value: playbackModel.beatNumber

                onValueChanged: {
                    playbackModel.beatNumber = value
                }
            }

            StyledTextLabel {
                Layout.leftMargin: 20
                topPadding: 10

                text: playbackModel.tempo.noteSymbol

                font.family: ui.theme.musicalFont
                font.pixelSize: ui.theme.tabFont.pixelSize
                font.letterSpacing: 1

                lineHeightMode: Text.FixedHeight
            }

            StyledTextLabel {
                Layout.rightMargin: 20

                text: "= " + playbackModel.tempo.value
                font: timeField.font
            }

            SeparatorLine { orientation: Qt.Vertical }
        }

        StyledSlider {
            width: playbackActions.width
            visible: root.floating
            value: playbackModel.playPosition

            onMoved: {
                playbackModel.playPosition = value
            }
        }
    }
}
