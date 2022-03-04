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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Audio 1.0

MixerPanelSection {
    id: root

    headerTitle: qsTrc("playback", "Sound")

    Item {
        id: content

        height: inputResourceControl.height
        width: root.channelItemWidth

        property string accessibleName: (Boolean(root.needReadChannelName) ? channelItem.title + " " : "") + root.headerTitle

        visible: !channelItem.outputOnly

        AudioResourceControl {
            id: inputResourceControl

            anchors.horizontalCenter: parent.horizontalCenter
            height: 26

            supportsByPassing: false
            resourceItemModel: channelItem.inputResourceItem

            navigationPanel: channelItem.panel
            navigationRowStart: root.navigationRowStart
            accessibleName: content.accessibleName

            onTitleClicked: {
                if (channelItem.inputResourceItem) {
                    channelItem.inputResourceItem.requestToLaunchNativeEditorView()
                }
            }

            onNavigateControlIndexChanged: function(index) {
                root.navigateControlIndexChanged(index)
            }
        }
    }
}
