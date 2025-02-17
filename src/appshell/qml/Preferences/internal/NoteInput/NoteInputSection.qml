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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

import "../../internal"

BaseSection {
    id: root

    title: qsTrc("appshell/preferences", "Note input")

    property alias noteInputMethods: defaultNoteInputMethodDropdown.model
    property int defaultNoteInputMethod: 0

    property bool addAccidentalDotsArticulationsToNextNoteEntered: true
    property bool useNoteInputCursorInInputByDuration: false

    signal defaultNoteInputMethodChangeRequested(int method)
    signal addAccidentalDotsArticulationsToNextNoteEnteredChangeRequested(bool add)
    signal useNoteInputCursorInInputByDurationChangeRequested(bool use)

    ComboBoxWithTitle {
        id: defaultNoteInputMethodDropdown

        title: qsTrc("appshell/preferences", "Default input mode:")

        currentIndex: defaultNoteInputMethodDropdown.indexOfValue(root.defaultNoteInputMethod)

        navigation.name: "DefaultNoteInputMethodDropdown"
        navigation.panel: root.navigation
        navigation.row: 0

        onValueEdited: function(newIndex, newValue) {
            root.defaultNoteInputMethodChangeRequested(newValue)
        }
    }

    ComboBoxWithTitle {
        title: qsTrc("appshell/preferences", "Apply accidentals, augmentation dots, and articulations:")

        navigation.name: "AddAccidentalDotsArticulationsToNextNoteEnteredDropdown"
        navigation.panel: root.navigation
        navigation.row: 1

        model: [
            { text: qsTrc("appshell/preferences", "To next note entered"), value: 0 },
            { text: qsTrc("appshell/preferences", "To previous note entered"), value: 1 },
        ]

        currentIndex: root.addAccidentalDotsArticulationsToNextNoteEntered ? 0 : 1

        onValueEdited: function(newIndex, newValue) {
            root.addAccidentalDotsArticulationsToNextNoteEnteredChangeRequested(newIndex === 0)
        }
    }

    ComboBoxWithTitle {
        title: qsTrc("appshell/preferences", "Input by duration mode cursor:")

        navigation.name: "InputByDurationModeCursorDropdown"
        navigation.panel: root.navigation
        navigation.row: 2

        model: [
            { text: qsTrc("appshell/preferences", "Above staff"), value: 0 },
            { text: qsTrc("appshell/preferences", "On staff"), value: 1 },
        ]

        currentIndex: root.useNoteInputCursorInInputByDuration ? 1 : 0

        onValueEdited: function(newIndex, newValue) {
            root.useNoteInputCursorInInputByDurationChangeRequested(newIndex === 1)
        }
    }
}
