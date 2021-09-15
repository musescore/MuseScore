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

import MuseScore.UiComponents 1.0

BaseSection {
    id: root

    property alias playNotesWhenEditing: playNotesBox.checked
    property alias playChordWhenEditing: playChordBox.checked
    property alias playChordSymbolWhenEditing: playChordSymbolBox.checked
    property alias notePlayDurationMilliseconds: notePlayDurationControl.currentValue

    signal playNotesWhenEditingChangeRequested(bool play)
    signal playChordWhenEditingChangeRequested(bool play)
    signal playChordSymbolWhenEditingChangeRequested(bool play)
    signal notePlayDurationChangeRequested(int duration)

    CheckBox {
        id: playNotesBox

        text: qsTrc("appshell", "Play notes when editing")
        font: ui.theme.bodyBoldFont

        onClicked: {
            root.playNotesWhenEditingChangeRequested(!checked)
        }
    }

    IncrementalPropertyControlWithTitle {
        id: notePlayDurationControl

        title: qsTrc("appshell", "Default duration:")

        spacing: 126

        measureUnitsSymbol: qsTrc("appshell", "ms")

        onValueEdited: {
            root.notePlayDurationChangeRequested(newValue)
        }
    }

    CheckBox {
        id: playChordBox

        text: qsTrc("appshell", "Play chord when editing")

        enabled: root.playNotesWhenEditing

        onClicked: {
            root.playChordWhenEditingChangeRequested(!checked)
        }
    }

    CheckBox {
        id: playChordSymbolBox

        text: qsTrc("appshell", "Play chord symbol when editing")

        enabled: root.playNotesWhenEditing

        onClicked: {
            root.playChordSymbolWhenEditingChangeRequested(!checked)
        }
    }
}
