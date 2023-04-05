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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../common"

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "TempoRestorePreviousSettings"

    spacing: 12

    function focusOnFirst() {
        setSpecificTempoCheckBox.navigation.requestActive()
    }

    CheckBox {
        id: setSpecificTempoCheckBox
        property PropertyItem followText: root.model ? root.model.followText : null

        text: qsTrc("inspector", "Set specific tempo")

        width: parent.width

        visible: followText && followText.isVisible
        enabled: followText && followText.isEnabled

        isIndeterminate: followText && followText.isUndefined
        checked: !isIndeterminate && followText && !Boolean(followText.value)
        onClicked: {
            if (followText) {
                followText.value = checked
            }
        }
        navigation.name: "SetSpecificTempoCheckBox"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRowStart + 1
    }

    SpinBoxPropertyView {
        titleText: qsTrc("inspector", "Specific tempo")
        propertyItem: root.model ? root.model.tempo : null
        enabled: root.model ? !root.model.isEmpty && setSpecificTempoCheckBox.checked : false

        measureUnitsSymbol: qsTrc("inspector", "BPM")

        navigationName: "Override"
        navigationPanel: root.navigationPanel
        navigationRowStart: setSpecificTempoCheckBox.navigation.row + 1
    }
}
