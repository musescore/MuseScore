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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Audio 1.0
import MuseScore.Playback 1.0

MixerPanelSection {
    id: root

    headerTitle: qsTrc("playback", "Name")

    Rectangle {
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

        MouseArea{
            id: mouseArea
            anchors.fill: parent

            hoverEnabled: true

            onContainsMouseChanged: {
                if (mouseArea.containsMouse) {
                    ui.tooltip.show(root, channelItem.title, "", "")
                } else {
                    ui.tooltip.hide(root)
                }
            }
        }

        StyledTextLabel {
            anchors.centerIn: parent

            font: ui.theme.bodyBoldFont

            readonly property int margin: -8
            width: margin + parent.width + margin

            text: channelItem.title
        }
    }
}
