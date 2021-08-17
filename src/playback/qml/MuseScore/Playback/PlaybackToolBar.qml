/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
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

    property alias navigation: navPanel
    property bool floating: false

    color: ui.theme.backgroundPrimaryColor

    NavigationPanel {
        id: navPanel
        name: "PlaybackToolBar"
        enabled: root.enabled && root.visible
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

                contentHeight: root.floating ? 32 : 48
                spacing: 4

                model: playbackModel

                orientation: Qt.Horizontal
                interactive: false

                delegate: Loader {
                    id: itemLoader
                    anchors.verticalCenter: parent ? parent.verticalCenter : undefined

                    sourceComponent: Boolean(model.code) || model.subitems.length !== 0 ? menuItemComp : separatorComp

                    onLoaded: {
                        itemLoader.item.modelData = model
                    }

                    Component {
                        id: menuItemComp

                        FlatButton {
                            id: btn

                            property var modelData
                            property bool hasSubitems: modelData.subitems.length !== 0

                            icon: modelData.icon

                            toolTipTitle: modelData.title
                            toolTipDescription: modelData.description
                            toolTipShortcut: modelData.shortcut
                            accessible.name: modelData.title
                            accessible.description: modelData.description;

                            iconFont: ui.theme.toolbarIconsFont

                            normalStateColor: modelData.checked || menuLoader.isMenuOpened
                                              ? ui.theme.accentColor : "transparent"
                            accentButton: modelData.checked || menuLoader.isMenuOpened

                            navigation.panel: navPanel
                            navigation.name: modelData.title
                            navigation.order: modelData.index
                            navigation.enabled: playbackModel.isPlayAllowed

                            onClicked: {
                                if (menuLoader.isMenuOpened || hasSubitems) {
                                    menuLoader.toggleOpened(modelData.subitems, btn.navigation)
                                    return
                                }

                                Qt.callLater(playbackModel.handleMenuItem, modelData.id)
                            }

                            StyledMenuLoader {
                                id: menuLoader
                                onHandleMenuItem: playbackModel.handleMenuItem(itemId)
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
