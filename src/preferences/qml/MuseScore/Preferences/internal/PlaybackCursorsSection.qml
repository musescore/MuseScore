/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

BaseSection {
    id: root

    property alias isShowingPlaybackCueInNormalMode: showPlaybackCueInNormalModeCheckbox.checked
    property alias isShowingPlaybackCursorInNormalMode: showPlaybackCursorInNormalModeCheckbox.checked

    property alias playbackCueColor: playbackCueColorPicker.color
    property alias playbackCursorColor: playbackCursorColorPicker.color

    signal showPlaybackCueInNormalModeChangeRequested(bool show)
    signal showPlaybackCursorInNormalModeChangeRequested(bool show)
    signal playbackCueColorChangeRequested(color newColor)
    signal playbackCursorColorChangeRequested(color newColor)

    navigation.direction: NavigationPanel.Both

    title: qsTrc("preferences", "Playback cursors")

    CheckBox {
        id: showPlaybackCueInNormalModeCheckbox
        width: parent.width

        text: qsTrc("preferences", "Show playback cue in normal mode")

        navigation.name: "ShowPlaybackCueInNormalModeCheckbox"
        navigation.panel: root.navigation
        navigation.row: 0

        onClicked: {
            root.showPlaybackCueInNormalModeChangeRequested(!checked)
        }
    }

    CheckBox {
        id: showPlaybackCursorInNormalModeCheckbox
        width: parent.width

        text: qsTrc("preferences", "Show playback cursor in normal mode")

        navigation.name: "ShowPlaybackCursorInNormalModeCheckbox"
        navigation.panel: root.navigation
        navigation.row: 1

        onClicked: {
            root.showPlaybackCursorInNormalModeChangeRequested(!checked)
        }
    }

    GridLayout {
        columns: 2
        columnSpacing: root.columnSpacing
        rowSpacing: root.rowSpacing

        StyledTextLabel {
            id: playbackCueColorLabel
            Layout.preferredWidth: root.columnWidth
            text: qsTrc("preferences", "Playback cue color")
            horizontalAlignment: Text.AlignLeft
        }

        ColorPicker {
            id: playbackCueColorPicker
            width: 112

            navigation.name: "PlaybackCueColorPicker"
            navigation.panel: root.navigation
            navigation.row: 2
            navigation.accessible.name: playbackCueColorLabel.text

            allowAlpha: true

            onNewColorSelected: function(newColor: color) {
                root.playbackCueColorChangeRequested(newColor)
            }
        }

        StyledTextLabel {
            id: playbackCursorColorLabel
            Layout.preferredWidth: root.columnWidth
            text: qsTrc("preferences", "Playback cursor color")
            horizontalAlignment: Text.AlignLeft
        }

        ColorPicker {
            id: playbackCursorColorPicker
            width: 112

            navigation.name: "PlaybackCursorColorPicker"
            navigation.panel: root.navigation
            navigation.row: 3
            navigation.accessible.name: playbackCursorColorLabel.text

            allowAlpha: true

            onNewColorSelected: function(newColor: color) {
                root.playbackCursorColorChangeRequested(newColor)
            }
        }
    }
}
