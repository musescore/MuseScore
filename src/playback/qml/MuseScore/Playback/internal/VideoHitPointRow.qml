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

RowLayout {
    id: root

    // { id, label, timeMs, timecode, musicalPosition, color }
    required property var hitPoint
    required property VideoPanelModel videoModel
    // function(videoPositionMs) -- seeks both the video element and the score
    required property var seekToVideoPositionMs

    required property NavigationPanel navigationPanel
    required property int navigationOrderStart

    property bool editingLabel: false
    property bool editingTimecode: false

    spacing: 8

    Item {
        Layout.preferredWidth: 88
        Layout.preferredHeight: 28

        StyledTextLabel {
            anchors.fill: parent
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            text: root.hitPoint.timecode
            maximumLineCount: 1
            color: ui.theme.accentColor
            visible: !root.editingTimecode

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor

                onClicked: root.seekToVideoPositionMs(root.hitPoint.timeMs)
                onDoubleClicked: {
                    root.editingTimecode = true
                    timecodeEditor.forceActiveFocus()
                    timecodeEditor.selectAll()
                }
            }
        }

        TextInputField {
            id: timecodeEditor

            anchors.fill: parent
            currentText: root.hitPoint.timecode
            visible: root.editingTimecode
            navigation.panel: root.navigationPanel
            navigation.order: root.navigationOrderStart

            onTextEditingFinished: function(newTextValue) {
                root.videoModel.setHitPointTimecode(root.hitPoint.id, newTextValue)
                root.editingTimecode = false
            }

            Keys.onEscapePressed: {
                root.editingTimecode = false
            }
        }
    }

    StyledTextLabel {
        Layout.preferredWidth: 64
        horizontalAlignment: Text.AlignRight
        text: root.hitPoint.musicalPosition
        maximumLineCount: 1
    }

    Item {
        Layout.fillWidth: true
        Layout.minimumWidth: 80
        Layout.preferredHeight: 28

        StyledTextLabel {
            anchors.fill: parent
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            text: root.hitPoint.label
            maximumLineCount: 1
            visible: !root.editingLabel

            MouseArea {
                anchors.fill: parent
                onDoubleClicked: {
                    root.editingLabel = true
                    labelEditor.forceActiveFocus()
                    labelEditor.selectAll()
                }
            }
        }

        TextInputField {
            id: labelEditor

            anchors.fill: parent
            currentText: root.hitPoint.label
            visible: root.editingLabel
            navigation.panel: root.navigationPanel
            navigation.order: root.navigationOrderStart + 1

            onTextEditingFinished: function(newTextValue) {
                root.videoModel.renameHitPoint(root.hitPoint.id, newTextValue)
                root.editingLabel = false
            }

            Keys.onEscapePressed: {
                root.editingLabel = false
            }
        }
    }

    FlatButton {
        Layout.preferredWidth: 28
        Layout.preferredHeight: 28
        icon: IconCode.DELETE_TANK
        buttonType: FlatButton.IconOnly
        navigation.panel: root.navigationPanel
        navigation.order: root.navigationOrderStart + 2

        onClicked: {
            root.videoModel.removeHitPoint(root.hitPoint.id)
        }
    }
}
