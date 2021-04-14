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

    KeyNavigationSubSection {
        id: keynavSub
        name: "PlaybackToolBar"
    }

    PlaybackToolBarModel {
        id: playbackModel
        isToolbarFloating: root.floating
    }

    Component.onCompleted: {
        playbackModel.load()
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

                model: playbackModel

                orientation: Qt.Horizontal
                interactive: false

                delegate: Loader {
                    id: itemLoader

                    sourceComponent: Boolean(model.code) || model.subitems.length !== 0 ? menuItemComp : separatorComp

                    onLoaded: {
                        itemLoader.item.modelData = model
                    }

                    Component {
                        id: menuItemComp

                        FlatButton {
                            property var modelData
                            property var hasSubitems: modelData.subitems.length !== 0

                            icon: modelData.icon
                            hint: modelData.hint
                            iconFont: ui.theme.toolbarIconsFont

                            normalStateColor: modelData.checked || menuLoader.isMenuOpened()
                                              ? ui.theme.accentColor : "transparent"
                            accentButton: modelData.checked || menuLoader.isMenuOpened()

                            keynav.subsection: keynavSub
                            keynav.name: modelData.hint
                            keynav.order: modelData.index
                            keynav.enabled: playbackModel.isPlayAllowed

                            onClicked: {
                                if (menuLoader.isMenuOpened() || hasSubitems) {
                                    menuLoader.toggleOpened(modelData.subitems)
                                    return
                                }

                                Qt.callLater(playbackModel.handleAction, modelData.code)
                            }

                            StyledMenuLoader {
                                id: menuLoader
                                onHandleAction: playbackModel.handleAction(actionCode)
                            }
                        }
                    }

                    Component {
                        id: separatorComp

                        SeparatorLine {
                            property var modelData
                            orientation: Qt.Vertical
                        }
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
