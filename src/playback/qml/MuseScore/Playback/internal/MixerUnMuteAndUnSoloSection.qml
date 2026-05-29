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

Item {
    id: root

    property var model: undefined
    property int headerWidth: 98
    property bool headerVisible: true
    property alias unMuteButton: unMuteAllButton
    property alias unSoloButton: unSoloAllButton

    height: 28
    width: headerWidth
    visible: headerVisible

    Row {
        anchors.centerIn: parent
        spacing: 6

        FlatToggleButton {
            id: unMuteAllButton

            height: 20
            width: 20

            icon: IconCode.MUTE

            enabled: parent.enabled
            visible: parent.visible

            navigation.name: "UnMuteAllButton"
            navigation.panel: root.channelItem.panel
            navigation.row: root.navigationRowStart
            navigation.accessible.name: root.accessibleName + " " + qsTrc("playback", "Unmute All")
            navigation.onActiveChanged: {
                if (navigation.active) {
                    root.navigateControlIndexChanged({row: navigation.row, column: navigation.column})
                }
            }

            onToggled: {
                for (let i = 0; i < model.rowCount(); i++) {
                    let item = model.get(i);

                    if(item.channelItem.type === MixerChannelItem.Metronome){
                        continue
                    }

                    item.channelItem.muted = false
                }
            }
        }

        FlatToggleButton {
            id: unSoloAllButton

            height: 20
            width: 20

            icon: IconCode.SOLO

            enabled: parent.enabled
            visible: parent.visible

            navigation.name: "UnSoloAllButton"
            navigation.panel: root.channelItem.panel
            navigation.row: root.navigationRowStart + 1
            navigation.accessible.name: root.accessibleName + " " + qsTrc("playback", "Unsolo All")
            navigation.onActiveChanged: {
                if (navigation.active) {
                    root.navigateControlIndexChanged({row: navigation.row, column: navigation.column})
                }
            }

            onToggled: {
                for (let i = 0; i < model.rowCount(); i++) {
                    let item = model.get(i);
                    item.channelItem.solo = false
                }
            }
        }
    }
}
