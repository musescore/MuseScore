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

    Item {
        height: childrenRect.height
        width: root.delegateDefaultWidth

        Component.onCompleted: {
            root.navigationRowEnd = root.navigationRowStart + 2 // todo
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter

            spacing: 6

            FlatToggleButton {
                id: muteButton

                height: 20
                width: 20

                icon: IconCode.MUTE
                checked: item.muted

                navigation.name: "MuteButton"
                navigation.panel: item.panel
                navigation.row: root.navigationRowStart
                navigation.onActiveChanged: {
                    if (navigation.active) {
                        root.navigateControlNameChanged(navigation.name)
                    }
                }

                onToggled: {
                    item.muted = !item.muted
                }
            }

            FlatToggleButton {
                id: soloButton

                height: 20
                width: 20

                icon: IconCode.SOLO
                checked: item.solo

                navigation.name: "SoloButton"
                navigation.panel: item.panel
                navigation.row: root.navigationRowStart + 1
                navigation.onActiveChanged: {
                    if (navigation.active) {
                        root.navigateControlNameChanged(navigation.name)
                    }
                }

                onToggled: {
                    item.solo = !item.solo
                }
            }
        }
    }
}
