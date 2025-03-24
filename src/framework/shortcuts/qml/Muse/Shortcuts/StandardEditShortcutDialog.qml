/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
import Muse.Shortcuts 1.0

StyledDialogView {
    id: root

    title: qsTrc("shortcuts", "Enter shortcut sequence")

    contentWidth: 538
    contentHeight: 200

    margins: 20

    signal applyNewSequenceRequested(var newSequence, var conflictShortcutIndex)
    signal load(var currentShortcut, var allShortcuts)

    onNavigationActivateRequested: {
        editShortcutDialogContent.requestActive()
    }

    onLoad: function(currentShortcut, allShortcuts) {
        editShortcutModel.load(currentShortcut, allShortcuts)
    }

    EditShortcutModel {
        id: editShortcutModel

        onApplyNewSequenceRequested: function(newSequence, conflictShortcutIndex) {
            root.applyNewSequenceRequested(newSequence, conflictShortcutIndex)
        }
    }

    EditShortcutDialogContent {
        id: editShortcutDialogContent

        navigationSection: root.navigationSection

        headerText: qsTrc("shortcuts", "Define keyboard shortcut")

        //! NOTE: There's no need to actually clear the origin shortcut, we can simply hide it for aesthetic purposes...
        originShortcutText: !editShortcutModel.cleared ? editShortcutModel.originSequence : ""
        newShortcutText: editShortcutModel.newSequence
        informationText: editShortcutModel.conflictWarning

        onSaveRequested: {
            editShortcutModel.trySave()
            root.accept()
        }

        onCancelRequested: {
            root.reject()
        }

        onClearRequested: {
            editShortcutModel.clear()
        }

        onKeyPressed: function(event) {
            editShortcutModel.inputKey(event.key, event.modifiers)
        }
    }
}
