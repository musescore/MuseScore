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
import QtQuick.Controls 2.15

import Muse.UiComponents 1.0

BaseSection {
    id: root

    title: qsTrc("appshell/preferences", "Automatic panning and scrolling")

    property alias autoPanInInputMode: autoPanInInputModeBox.checked
    property alias autoPanDuringPlayback: autoPanDuringPlaybackBox.checked
    property alias autoPanOtherwise: autoPanOtherwiseBox.checked

    signal autoPanInInputModeChangeRequested(bool autoPan)
    signal autoPanDuringPlaybackChangeRequested(bool autoPan)
    signal autoPanOtherwiseChangeRequested(bool autoPan)

    CheckBox {
        id: autoPanInInputModeBox
        width: parent.width

        text: qsTrc("appshell/preferences", "Pan and scroll score automatically in note/text input mode")

        navigation.name: "AutoPanInInputModeBox"
        navigation.panel: root.navigation
        navigation.row: 0

        onClicked: {
            root.autoPanInInputModeChangeRequested(!checked)
        }
    }

    CheckBox {
        id: autoPanDuringPlaybackBox
        width: parent.width

        text: qsTrc("appshell/preferences", "Pan and scroll score automatically during playback")

        navigation.name: "AutoPanDuringPlaybackBox"
        navigation.panel: root.navigation
        navigation.row: 1

        onClicked: {
            root.autoPanDuringPlaybackChangeRequested(!checked)
        }
    }
    
    CheckBox {
        id: autoPanOtherwiseBox
        width: parent.width

        text: qsTrc("appshell/preferences", "Pan and scroll score automatically otherwise")

        navigation.name: "AutoPanOtherwiseBox"
        navigation.panel: root.navigation
        navigation.row: 2

        onClicked: {
            root.autoPanOtherwiseChangeRequested(!checked)
        }
    }
}
