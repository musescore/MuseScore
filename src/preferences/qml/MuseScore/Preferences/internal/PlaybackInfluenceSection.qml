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
import QtQuick

import Muse.Ui
import Muse.UiComponents
import MuseScore.Preferences

BaseSection {
    id: root

    title: qsTrc("preferences", "Note playback info")

    navigation.direction: NavigationPanel.Both

    property ScorePreferencesModel model: null

    readonly property string helpText: qsTrc("preferences/score", "Highlights score elements which affect the selected note during playback.\nShown in the Playback section of the Properties Panel or while its shortcut keys are held down (see Shortcuts)")

    // Anywhere over the section, title included; a HoverHandler doesn't take the mouse from the controls in it.
    // The tooltip is attached to the label: it puts its popup inside the item it's shown on, which breaks a Column.
    HoverHandler {
        parent: root

        onHoveredChanged: {
            if (hovered) {
                ui.tooltip.show(highlightColorLabel, root.helpText)
            } else {
                ui.tooltip.hide(highlightColorLabel)
            }
        }
    }

    Row {
        spacing: root.columnSpacing

        StyledTextLabel {
            id: highlightColorLabel

            width: root.columnWidth
            anchors.verticalCenter: parent.verticalCenter

            text: qsTrc("preferences/score", "Highlight color")
            horizontalAlignment: Text.AlignLeft
        }

        ColorPicker {
            id: highlightColorPicker

            width: 112

            color: root.model ? root.model.playbackInfluenceHighlightColor : "transparent"

            // The default is see-through, so the notes under a highlighted item still show
            allowAlpha: true

            navigation.name: "PlaybackInfluenceHighlightColorBox"
            navigation.panel: root.navigation
            navigation.row: 0
            navigation.column: 1
            navigation.accessible.name: highlightColorLabel.text + " " + Utils.accessibleColorDescription(highlightColorPicker.color)
            navigation.accessible.description: root.helpText

            onNewColorSelected: function(newColor) {
                if (root.model) {
                    root.model.playbackInfluenceHighlightColor = newColor
                }
            }
        }
    }
}
