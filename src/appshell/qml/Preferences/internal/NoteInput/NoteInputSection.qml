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

    signal defaultNoteInputMethodChangeRequested(int method)

    ComboBoxWithTitle {
        id: defaultNoteInputMethodDropdown

        title: qsTrc("appshell/preferences", "Default input mode")

        currentIndex: defaultNoteInputMethodDropdown.indexOfValue(root.defaultNoteInputMethod)

        navigation.name: "DefaultNoteInputMethodDropdown"
        navigation.panel: root.navigation
        navigation.row: 0

        onValueEdited: function(newIndex, newValue) {
            root.defaultNoteInputMethodChangeRequested(newValue)
        }
    }
}
