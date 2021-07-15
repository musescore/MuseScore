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
import MuseScore.Inspector 1.0

Column {
    id: root

    property QtObject model: null

    objectName: "ClefSettings"

    spacing: 12

    CheckBox {
        isIndeterminate: root.model ? root.model.shouldShowCourtesy.isUndefined : false
        checked: root.model && !isIndeterminate ? root.model.shouldShowCourtesy.value : false
        text: qsTrc("inspector", "Show courtesy clef on previous measure")

        onClicked: { root.model.shouldShowCourtesy.value = !checked }
    }
}
