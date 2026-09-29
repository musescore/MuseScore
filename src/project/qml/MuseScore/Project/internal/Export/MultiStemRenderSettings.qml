/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

import Muse.UiComponents
import Muse.Ui
import MuseScore.Project

Column {
    id: root

    property ExportDialogModel model
    property NavigationPanel navigationPanel: null
    property int navigationOrder: 0

    width: parent ? parent.width : implicitWidth
    spacing: 8

    CheckBox {
        id: multiStemRenderCheckBox

        width: parent.width
        text: qsTrc("project/export", "Multi-stem render")

        navigation.name: "MultiStemRenderCheckbox"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationOrder

        checked: root.model ? root.model.multiStemRender : false

        onClicked: {
            root.model.multiStemRender = !checked
        }

        onHoveredChanged: {
            if (hovered) {
                ui.tooltip.show(multiStemRenderCheckBox, qsTrc("project/export", "Multi-stem render"),
                                qsTrc("project/export", "Renders all selected parts at the same time. Can be much faster for large arrangements. Aux effects (such as reverb) are copied for each part. Turn this off if a plugin sounds different or fails to load during export."))
            } else {
                ui.tooltip.hide(multiStemRenderCheckBox)
            }
        }
    }

    CheckBox {
        id: idleUntilFirstNoteCheckBox

        x: 24
        width: parent.width - x
        enabled: multiStemRenderCheckBox.checked
        text: qsTrc("project/export", "Idle instruments until their first note")

        navigation.name: "IdleUntilFirstNoteCheckbox"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationOrder + 1

        checked: root.model ? root.model.idleUntilFirstNote : false

        onClicked: {
            root.model.idleUntilFirstNote = !checked
        }

        onHoveredChanged: {
            if (hovered) {
                ui.tooltip.show(idleUntilFirstNoteCheckBox, qsTrc("project/export", "Idle instruments until their first note"),
                                qsTrc("project/export", "Skips processing each instrument until shortly before its first note, since it’s silent until then. Turn this off for instruments that make sound without notes, such as drones or noise generators."))
            } else {
                ui.tooltip.hide(idleUntilFirstNoteCheckBox)
            }
        }
    }
}
