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
import QtQuick 2.15

import Muse.Ui 1.0
import Muse.UiComponents
import Muse.Audio 1.0
import MuseScore.Playback 1.0

MixerPanelSection {
    id: root

    headerTitle: qsTrc("playback", "Name")

    signal menuButtonClicked(int channelIndex, var anchorItem)

    Rectangle {
        id: titleRect
        width: root.channelItemWidth
        height: 22

        property bool isInstrumentChannel: channelItem.type === MixerChannelItem.PrimaryInstrument ||
                                           channelItem.type === MixerChannelItem.SecondaryInstrument

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
            anchors.left: parent.left
            anchors.right: menuButton.visible ? menuButton.left : parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 4
            anchors.rightMargin: 2

            font: ui.theme.bodyBoldFont
            horizontalAlignment: Text.AlignHCenter

            text: channelItem.title
        }

        FlatButton {
            id: menuButton
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 2

            visible: titleRect.isInstrumentChannel
            width: 18
            height: 18

            icon: IconCode.MENU_THREE_DOTS
            iconFont: ui.theme.toolbarIconsFont
            transparent: true

            onClicked: {
                root.menuButtonClicked(model.index, menuButton)
            }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            z: -1

            enabled: parent.enabled
            hoverEnabled: true

            onContainsMouseChanged: {
                if (mouseArea.containsMouse && textLabel.truncated) {
                    ui.tooltip.show(mouseArea, channelItem.title)
                } else {
                    ui.tooltip.hide(mouseArea)
                }
            }
        }
    }
}
