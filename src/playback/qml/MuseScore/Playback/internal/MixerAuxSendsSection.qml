/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
import Muse.UiComponents 1.0
import Muse.Audio 1.0

MixerPanelSection {
    id: root

    headerTitle: qsTrc("playback", "Aux sends")
    headerHeight: 24

    Column {
        id: content

        y: 0

        height: childrenRect.height
        width: root.channelItemWidth

        property string accessibleName: (Boolean(root.needReadChannelName) ? channelItem.title + " " : "") + root.headerTitle

        spacing: 4

        Repeater {
            id: repeater
            anchors.horizontalCenter: parent.horizontalCenter

            model: channelItem.auxSendItemList

            delegate: AuxSendControl {
                id: auxSendControl

                anchors.horizontalCenter: parent.horizontalCenter

                auxSendItemModel: modelData

                navigationPanel: channelItem.panel
                navigationRowStart: root.navigationRowStart + (model.index * 2) // NOTE: 2 - because AuxSendControl has 2 controls
                navigationName: content.accessibleName
                accessibleName: content.accessibleName

                onNavigateControlIndexChanged: function(index) {
                    root.navigateControlIndexChanged(index)
                }
            }
        }
    }
}
