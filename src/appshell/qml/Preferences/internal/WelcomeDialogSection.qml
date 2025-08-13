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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Preferences

BaseSection {
    id: root

    title: qsTrc("appshell/preferences", "Welcome screen")

    required property GeneralPreferencesModel model

    rowSpacing: 16

    CheckBox {
        id: checkbox

        width: parent.width

        text: qsTrc("appshell/preferences", "Show welcome screen when MuseScore Studio launches")
        checked: root.model.showWelcomeDialog

        navigation.name: checkbox.text
        navigation.panel: root.navigation

        onClicked: {
            root.model.showWelcomeDialog = !root.model.showWelcomeDialog
        }
    }
}
