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

    function colorFromInt(value) {
        return "#" + value.toString(16).padStart(6, "0")
    }

    spacing: 8

    Item {
        Layout.preferredWidth: 108
        Layout.preferredHeight: 28

        RowLayout {
            anchors.fill: parent
            spacing: 2
            visible: !root.editingTimecode

            StyledTextLabel {
                Layout.fillWidth: true
                verticalAlignment: Text.AlignVCenter
                text: root.hitPoint.timecode
                maximumLineCount: 1
                color: root.colorFromInt(root.hitPoint.color)

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.seekToVideoPositionMs(root.hitPoint.timeMs)
                }
            }

            FlatButton {
                Layout.preferredWidth: 18
                Layout.preferredHeight: 18
                icon: IconCode.EDIT
                buttonType: FlatButton.IconOnly
                transparent: true
                toolTipTitle: qsTrc("playback", "Edit timecode")

                onClicked: {
                    root.editingTimecode = true
                    timecodeEditor.forceActiveFocus()
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
        Layout.preferredWidth: 56
        text: root.hitPoint.musicalPosition
        maximumLineCount: 1
    }

    Item {
        Layout.fillWidth: true
        Layout.preferredHeight: 28

        RowLayout {
            anchors.fill: parent
            spacing: 2
            visible: !root.editingLabel

            StyledTextLabel {
                Layout.fillWidth: true
                verticalAlignment: Text.AlignVCenter
                text: root.hitPoint.label
                maximumLineCount: 1
            }

            FlatButton {
                Layout.preferredWidth: 18
                Layout.preferredHeight: 18
                icon: IconCode.EDIT
                buttonType: FlatButton.IconOnly
                transparent: true
                toolTipTitle: qsTrc("playback", "Rename hit point")

                onClicked: {
                    root.editingLabel = true
                    labelEditor.forceActiveFocus()
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
