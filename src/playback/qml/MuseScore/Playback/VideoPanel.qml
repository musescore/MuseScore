/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.Playback

Item {
    id: root

    property NavigationSection navigationSection: null
    property int contentNavigationPanelOrderStart: 0

    VideoPanelModel {
        id: videoModel
    }

    Component.onCompleted: {
        videoModel.load()
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 16

        Rectangle {
            Layout.preferredWidth: 280
            Layout.fillHeight: true
            Layout.minimumHeight: 136

            radius: 4
            color: ui.theme.backgroundSecondaryColor
            border.width: ui.theme.borderWidth
            border.color: ui.theme.strokeColor

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                StyledIconLabel {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 12
                    iconCode: IconCode.PLAY
                    font.pixelSize: 34
                    opacity: videoModel.hasVideo ? 1.0 : 0.45
                }

                StyledTextLabel {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    text: videoModel.hasVideo ? videoModel.videoPath : qsTrc("playback", "No video attached")
                    maximumLineCount: 3
                    wrapMode: Text.WrapAnywhere
                    displayTruncatedTextOnHover: true
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            FilePicker {
                Layout.fillWidth: true
                path: videoModel.videoPath
                dialogTitle: qsTrc("playback", "Choose video")
                filter: qsTrc("playback", "Video files (*.mp4 *.mov *.m4v *.avi *.mkv *.webm);;All files (*)")
                buttonType: FlatButton.Horizontal
                navigation: navigationPanel
                navigationRowOrderStart: root.contentNavigationPanelOrderStart

                onPathEdited: function(newPath) {
                    videoModel.videoPath = newPath
                }
            }

            RowLayout {
                spacing: 8

                StyledTextLabel {
                    text: qsTrc("playback", "Offset")
                }

                FlatButton {
                    text: qsTrc("playback", "-100 ms")
                    enabled: videoModel.hasVideo
                    navigation.panel: navigationPanel
                    navigation.order: root.contentNavigationPanelOrderStart + 2

                    onClicked: {
                        videoModel.nudgeOffset(-100)
                    }
                }

                TextInputField {
                    Layout.preferredWidth: 88
                    currentText: videoModel.offsetMs.toString()
                    enabled: videoModel.hasVideo
                    navigation.panel: navigationPanel
                    navigation.order: root.contentNavigationPanelOrderStart + 3

                    onTextEditingFinished: function(newTextValue) {
                        var parsedValue = parseInt(newTextValue)
                        if (!isNaN(parsedValue)) {
                            videoModel.offsetMs = parsedValue
                        }
                    }
                }

                StyledTextLabel {
                    text: qsTrc("playback", "ms")
                }

                FlatButton {
                    text: qsTrc("playback", "+100 ms")
                    enabled: videoModel.hasVideo
                    navigation.panel: navigationPanel
                    navigation.order: root.contentNavigationPanelOrderStart + 4

                    onClicked: {
                        videoModel.nudgeOffset(100)
                    }
                }
            }

            RowLayout {
                spacing: 12

                CheckBox {
                    text: qsTrc("playback", "Mute")
                    checked: videoModel.muted
                    enabled: videoModel.hasVideo
                    navigation.panel: navigationPanel
                    navigation.order: root.contentNavigationPanelOrderStart + 5

                    onClicked: {
                        videoModel.muted = !videoModel.muted
                    }
                }

                StyledTextLabel {
                    text: qsTrc("playback", "Volume")
                    enabled: videoModel.hasVideo
                }

                StyledSlider {
                    Layout.preferredWidth: 180
                    from: 0
                    to: 100
                    stepSize: 1
                    value: videoModel.volumePercent
                    enabled: videoModel.hasVideo && !videoModel.muted
                    navigation.panel: navigationPanel
                    navigation.order: root.contentNavigationPanelOrderStart + 6

                    onMoved: {
                        videoModel.volumePercent = value
                    }
                }

                StyledTextLabel {
                    text: videoModel.volumePercent + "%"
                    enabled: videoModel.hasVideo
                }
            }

            RowLayout {
                Layout.fillWidth: true

                Item {
                    Layout.fillWidth: true
                }

                FlatButton {
                    text: qsTrc("playback", "Clear")
                    enabled: videoModel.hasVideo
                    navigation.panel: navigationPanel
                    navigation.order: root.contentNavigationPanelOrderStart + 7

                    onClicked: {
                        videoModel.clearVideo()
                    }
                }
            }
        }
    }

    NavigationPanel {
        id: navigationPanel
        name: "VideoPanel"
        section: root.navigationSection
        order: root.contentNavigationPanelOrderStart
        direction: NavigationPanel.Vertical
    }
}
