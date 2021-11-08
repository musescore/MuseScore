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
import QtQuick.Layouts 1.15

import MuseScore.Playback 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.CommonScene 1.0

import "internal"

Rectangle {
    id: root

    property alias navigation: navPanel
    property bool floating: false

    property int maximumHeight: 0

    width: floating ? 426 : 364 //content.width
    height: content.height

    color: ui.theme.backgroundPrimaryColor

    NavigationPanel {
        id: navPanel
        name: "PlaybackToolBar"
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("playback", "Playback toolbar")
    }

    PlaybackToolBarModel {
        id: playbackModel
        isToolbarFloating: root.floating
    }

    Component.onCompleted: {
        playbackModel.load()
    }

    Column {
        id: content

        spacing: 14

        width: childrenRect.width

        enabled: playbackModel.isPlayAllowed

        RowLayout {
            id: playbackActions

            spacing: 0

            ListView {
                id: buttonsListView
                Layout.preferredWidth: contentItem.childrenRect.width
                Layout.preferredHeight: contentItem.childrenRect.height

                contentHeight: 30
                spacing: 4

                model: playbackModel

                orientation: Qt.Horizontal
                interactive: false

                delegate: FlatButton {
                    id: btn

                    width: 30
                    height: width

                    icon: model.icon

                    toolTipTitle: model.title
                    toolTipDescription: model.description
                    toolTipShortcut: model.shortcut

                    iconFont: ui.theme.toolbarIconsFont

                    accentButton: model.checked || menuLoader.isMenuOpened
                    transparent: !accentButton

                    navigation.panel: navPanel
                    navigation.name: model.title
                    navigation.order: model.index

                    onClicked: {
                        if (menuLoader.isMenuOpened || model.subitems.length) {
                            menuLoader.toggleOpened(model.subitems)
                            return
                        }

                        Qt.callLater(playbackModel.handleMenuItem, model.id)
                    }

                    StyledMenuLoader {
                        id: menuLoader

                        navigation: btn.navigation

                        onHandleMenuItem: {
                            playbackModel.handleMenuItem(itemId)
                        }
                    }
                }
            }

            SeparatorLine {
                Layout.topMargin: 2
                Layout.bottomMargin: 2
                Layout.leftMargin: 6

                orientation: Qt.Vertical
                visible: !root.floating
            }

            RowLayout {
                Layout.leftMargin: 12
                Layout.rightMargin: root.floating ? 12 : 0
                spacing: 18

                TimeInputField {
                    id: timeField

                    maxTime: playbackModel.maxPlayTime
                    maxMillisecondsNumber: 9
                    time: playbackModel.playTime

                    onTimeEdited: function(newTime) {
                        playbackModel.playTime = newTime
                    }
                }

                MeasureAndBeatFields {
                    measureNumber: playbackModel.measureNumber
                    maxMeasureNumber: playbackModel.maxMeasureNumber
                    beatNumber: playbackModel.beatNumber
                    maxBeatNumber: playbackModel.maxBeatNumber

                    font: timeField.font

                    onMeasureNumberEdited: function(newValue) {
                        playbackModel.measureNumber = newValue
                    }

                    onBeatNumberEdited: function(newValue) {
                        playbackModel.beatNumber = newValue
                    }
                }

                TempoView {
                    noteSymbol: playbackModel.tempo.noteSymbol
                    tempoValue: playbackModel.tempo.value

                    noteSymbolFont.pixelSize: ui.theme.iconsFont.pixelSize
                    tempoValueFont: timeField.font
                }
            }

            SeparatorLine {
                Layout.topMargin: 2
                Layout.bottomMargin: 2
                Layout.leftMargin: 12

                orientation: Qt.Vertical
                visible: !root.floating
            }
        }

        StyledSlider {
            width: playbackActions.width - 12
            visible: root.floating
            value: playbackModel.playPosition

            onMoved: {
                playbackModel.playPosition = value
            }
        }
    }
}
