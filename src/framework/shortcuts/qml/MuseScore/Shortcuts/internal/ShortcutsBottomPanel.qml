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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Shortcuts 1.0

RowLayout {
    id: root

    property alias canResetCurrentShortcut: resetButton.enabled

    property int buttonWidth: 0

    signal importShortcutsFromFileRequested()
    signal exportShortcutsToFileRequested()
    signal resetToDefaultSelectedShortcuts()

    FlatButton {
        Layout.preferredWidth: root.buttonWidth

        text: qsTrc("shortcuts", "Import")

        onClicked: {
            root.importShortcutsFromFile()
        }
    }

    FlatButton {
        Layout.preferredWidth: root.buttonWidth

        text: qsTrc("shortcuts", "Export")

        onClicked: {
            root.exportShortcutsToFileRequested()
        }
    }

    Item { Layout.fillWidth: true }

    FlatButton {
        id: resetButton

        Layout.preferredWidth: root.buttonWidth

        text: qsTrc("shortcuts", "Reset to default")

        onClicked: {
            root.resetToDefaultSelectedShortcuts()
        }
    }
}
