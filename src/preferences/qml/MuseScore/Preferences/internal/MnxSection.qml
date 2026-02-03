/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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
import QtQuick

import Muse.Ui
import Muse.UiComponents

BaseSection {
    id: root

    title: qsTrc("preferences", "MNX")
    readonly property string requireExactSchemaValidationTooltip:
        qsTrc("preferences", "Disabling this may cause errors. Try only for files that fail to import otherwise.")

    property alias requireExactSchemaValidation: requireExactSchemaValidationBox.checked

    signal requireExactSchemaValidationChangeRequested(bool value)

    CheckBox {
        id: requireExactSchemaValidationBox
        width: parent.width

        text: qsTrc("preferences", "Require exact schema validation")
        navigation.accessible.description: root.requireExactSchemaValidationTooltip

        navigation.name: "MnxRequireExactSchemaValidationBox"
        navigation.panel: root.navigation
        navigation.row: 0

        onHoveredChanged: {
            if (hovered) {
                ui.tooltip.show(requireExactSchemaValidationBox, root.requireExactSchemaValidationTooltip)
            } else {
                ui.tooltip.hide(requireExactSchemaValidationBox)
            }
        }

        onPressedChanged: {
            if (pressed) {
                ui.tooltip.hide(requireExactSchemaValidationBox, true)
            }
        }

        onClicked: root.requireExactSchemaValidationChangeRequested(!checked)
    }
}
