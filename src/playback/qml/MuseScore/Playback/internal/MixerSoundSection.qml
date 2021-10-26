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
        width: root.delegateDefaultWidth

        property string accessibleName: (Boolean(root.needReadChannelName) ? item.title + " " : "") + root.headerTitle

        Component.onCompleted: {
            root.navigationRowEnd = inputResourceControl.navigationRowEnd
        }

        visible: !item.outputOnly

        AudioResourceControl {
            id: inputResourceControl

            anchors.horizontalCenter: parent.horizontalCenter

            menuAnchorItem: root.rootPanel
            supportsByPassing: false
            resourceItemModel: item.inputResourceItem

            navigationPanel: item.panel
            navigationRowStart: root.navigationRowStart
            accessibleName: content.accessibleName

            onTitleClicked: {
                if (item.inputResourceItem) {
                    item.inputResourceItem.requestToLaunchNativeEditorView()
                }
            }

            onNavigateControlNameChanged: {
                root.navigateControlNameChanged(name)
            }
        }
    }
}
