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
import QtQuick.Controls
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.Playback

ExpandableBlank {
    id: root

    required property VideoPanelModel videoModel
    // function(videoPositionMs) -- seeks both the video element and the score
    required property var seekToVideoPositionMs
    // function() -> number (detected fps of the attached video, 0 if unknown)
    required property var detectFrameRate
    required property real currentVideoPositionMs

    required property NavigationPanel navigationPanel
    required property int navigationOrderStart

    readonly property int hitPointsRowStride: 3

    title: qsTrc("playback", "Hit points")
    titleFont: ui.theme.bodyBoldFont
    isExpanded: true
    enabled: videoModel.hasVideo

    navigation.panel: navigationPanel
    navigation.order: navigationOrderStart

    headerAccessory: Component {
        StyledTextLabel {
            text: root.videoModel.hitPoints.length.toString()
            opacity: 0.7
        }
    }

    contentItemComponent: ColumnLayout {
        width: root.width
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            StyledTextLabel {
                text: qsTrc("playback", "fps")
            }

            TextInputField {
                Layout.preferredWidth: 56
                currentText: root.videoModel.frameRate.toString()
                navigation.panel: root.navigationPanel
                navigation.order: root.navigationOrderStart + 1

                onTextEditingFinished: function(newTextValue) {
                    var parsedValue = parseFloat(newTextValue)
                    if (!isNaN(parsedValue)) {
                        root.videoModel.frameRate = parsedValue
                    }
                }
            }

            FlatButton {
                Layout.fillWidth: true
                text: qsTrc("playback", "Detect")
                enabled: root.detectFrameRate() > 0
                navigation.panel: root.navigationPanel
                navigation.order: root.navigationOrderStart + 2

                onClicked: {
                    var rate = root.detectFrameRate()
                    if (rate > 0) {
                        root.videoModel.frameRate = rate
                    }
                }
            }
        }

        FlatButton {
            Layout.fillWidth: true
            text: qsTrc("playback", "Add hit point")
            navigation.panel: root.navigationPanel
            navigation.order: root.navigationOrderStart + 3

            onClicked: {
                root.videoModel.addHitPoint(root.currentVideoPositionMs)
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            visible: root.videoModel.hitPoints.length > 0

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                StyledTextLabel {
                    Layout.preferredWidth: 88
                    text: qsTrc("playback", "Timecode")
                    font: ui.theme.bodyBoldFont
                    maximumLineCount: 1
                }

                StyledTextLabel {
                    Layout.preferredWidth: 56
                    text: qsTrc("playback", "Measure")
                    font: ui.theme.bodyBoldFont
                    maximumLineCount: 1
                }

                StyledTextLabel {
                    Layout.fillWidth: true
                    text: qsTrc("playback", "Name")
                    font: ui.theme.bodyBoldFont
                    maximumLineCount: 1
                }

                Item {
                    Layout.preferredWidth: 28
                }
            }

            StyledFlickable {
                id: hitPointsFlickable

                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(hitPointsColumn.implicitHeight, 220)

                contentHeight: hitPointsColumn.implicitHeight
                interactive: contentHeight > height
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                readonly property bool overflowing: contentHeight > height + 1

                ScrollBar.vertical: StyledScrollBar {
                    policy: hitPointsFlickable.overflowing ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
                    padding: 0
                }

                ColumnLayout {
                    id: hitPointsColumn

                    width: Math.max(0, hitPointsFlickable.width - (hitPointsFlickable.overflowing ? 12 : 0))
                    spacing: 4

                    Repeater {
                        model: root.videoModel.hitPoints

                        VideoHitPointRow {
                            id: hitPointRow

                            required property var modelData
                            required property int index

                            Layout.fillWidth: true

                            hitPoint: modelData
                            videoModel: root.videoModel
                            seekToVideoPositionMs: root.seekToVideoPositionMs
                            navigationPanel: root.navigationPanel
                            navigationOrderStart: root.navigationOrderStart + 4 + (hitPointRow.index * root.hitPointsRowStride)
                        }
                    }
                }
            }
        }
    }
}
