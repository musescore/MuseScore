import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import MuseScore.Playback 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.CommonScene 1.0

import "internal"

Rectangle {
    id: root

    property alias keynav: keynavSub
    property bool floating: false

    Component.onCompleted: {
        playbackModel.load()
    }

    KeyNavigationSubSection {
        id: keynavSub
        name: "PlaybackToolBar"
    }

    PlaybackToolBarModel {
        id: playbackModel
    }

    PlaybackSettingsPopup {
        id: playbackSettings
        onIsOpenedChanged: {
            if (!isOpened) {
                parent = root
            }
        }
    }

    Column {
        spacing: 14

        anchors.verticalCenter: parent.verticalCenter
        width: parent.width

        enabled: playbackModel.isPlayAllowed

        RowLayout {
            id: playbackActions

            spacing: 0

            ListView {
                Layout.preferredWidth: childrenRect.width
                Layout.preferredHeight: childrenRect.height

                contentHeight: 32
                contentWidth: contentHeight

                spacing: 4

                model: SortFilterProxyModel {
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
                    icon: model.icon
                    hint: model.hint
                    iconFont: ui.theme.toolbarIconsFont

                    keynav.subsection: keynavSub
                    keynav.order: model.index
                    keynav.enabled: playbackModel.isPlayAllowed

                    normalStateColor: (model.checked || (model.isPlaybackSettings && playbackSettings.isOpened)) ? ui.theme.accentColor : "transparent"

                    onClicked: {
                        if (model.isPlaybackSettings) {
                            playbackSettings.parent = this
                            playbackSettings.toggleOpened()
                            return
                        }

                        playbackModel.handleAction(model.code)
                    }
                }
            }

            SeparatorLine {
                Layout.leftMargin: 12
                orientation: Qt.Vertical
                visible: !root.floating
            }

            TimeInputField {
                id: timeField

                Layout.leftMargin: 24
                Layout.preferredWidth: 60

                maxTime: playbackModel.maxPlayTime
                maxMillisecondsNumber: 9
                time: playbackModel.playTime

                onTimeEdited: {
                    playbackModel.playTime = newTime
                }
            }

            MeasureAndBeatFields {
                Layout.leftMargin: 24

                measureNumber: playbackModel.measureNumber
                maxMeasureNumber: playbackModel.maxMeasureNumber
                beatNumber: playbackModel.beatNumber
                maxBeatNumber: playbackModel.maxBeatNumber

                font: timeField.font

                onMeasureNumberEdited: {
                    playbackModel.measureNumber = newValue
                }

                onBeatNumberEdited: {
                    playbackModel.beatNumber = newValue
                }
            }

            TempoView {
                Layout.leftMargin: 24
                Layout.preferredWidth: 60

                noteSymbol: playbackModel.tempo.noteSymbol
                tempoValue: playbackModel.tempo.value

                noteSymbolFont.pixelSize: ui.theme.iconsFont.pixelSize
                tempoValueFont: timeField.font
            }

            SeparatorLine {
                Layout.leftMargin: 24
                orientation: Qt.Vertical
                visible: !root.floating
            }
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
