/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import Muse.Ui
import Muse.UiComponents
import MuseScore.Playback

MixerPanelSection {
    id: root

    headerTitle: qsTrc("playback", "Name")

    Rectangle {
        id: content

        required property MixerChannelItem channelItem

        width: root.channelItemWidth
        height: 22

        function resolveLabelColor() {
            switch(channelItem.type) {
            case MixerChannelItem.PrimaryInstrument:
            case MixerChannelItem.SecondaryInstrument:
                return ui.theme.accentColor
            case MixerChannelItem.Aux:
                return "#63D47B"
            case MixerChannelItem.Master:
                return "#F87BDC"
            }

            return ui.theme.accentColor
        }

        function resolveLabelColorOpacity() {
            if (channelItem.type === MixerChannelItem.SecondaryInstrument) {
                return 0.25
            }

            return 0.5
        }

        readonly property color labelColor: resolveLabelColor()

        color: Utils.colorWithAlpha(labelColor, resolveLabelColorOpacity())
        border.color: labelColor
        border.width: 1

        StyledTextLabel {
            id: textLabel
            anchors.centerIn: parent

            font: ui.theme.bodyBoldFont

            readonly property int margin: -8
            width: margin + parent.width + margin

            text: content.channelItem.title
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent

            enabled: parent.enabled
            hoverEnabled: true

            onContainsMouseChanged: {
                if (mouseArea.containsMouse && textLabel.truncated) {
                    ui.tooltip.show(mouseArea, content.channelItem.title)
                } else {
                    ui.tooltip.hide(mouseArea)
                }
            }
        }
    }
}
