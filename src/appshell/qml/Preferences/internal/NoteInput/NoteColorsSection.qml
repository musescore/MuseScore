/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

import Muse.UiComponents 1.0

import "../../internal"

BaseSection {
    id: root

    property alias colorNotes: colorNotesBox.checked
    property alias warnGuitarBends: warnBendsBox.checked

    signal colorNotesChangeRequested(bool color)
    signal warnGuitarBendsChangeRequested(bool warn)

    title: qsTrc("appshell/preferences", "Note colors")

    CheckBox {
        id: colorNotesBox
        width: parent.width

        text: qsTrc("appshell/preferences", "Color notes outside of usable pitch range")

        navigation.name: "ColorNotesBox"
        navigation.panel: root.navigation
        navigation.row: 0

        onClicked: {
            root.colorNotesChangeRequested(!checked)
        }
    }

    CheckBox {
        id: warnBendsBox
        width: parent.width

        text: qsTrc("appshell/preferences", "Color guitar bends outside of playable range")

        navigation.name: "WarnBendBox"
        navigation.panel: root.navigation
        navigation.row: 1

        onClicked: {
            root.warnGuitarBendsChangeRequested(!checked)
        }
    }
}
