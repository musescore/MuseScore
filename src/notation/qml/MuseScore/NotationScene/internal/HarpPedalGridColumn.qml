/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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
import MuseScore.NotationScene 1.0

ColumnLayout {
    property var repeaterModel
    property QtObject model: harpModel

    property var buttonId: repeaterModel.buttonsId

    property NavigationPanel navPanel: null

    Layout.preferredWidth: 20
    Layout.preferredHeight: 120

    function checkPedalState(string, state) {
        var pedalState = harpModel.pedalState
        if (pedalState[string] == state) {
            return true
        }
        return false
    }

    function updatePedalState(string, state) {
        root.pedalState[string] = state
        harpModel.setDiagramPedalState(root.pedalState)
    }

    StyledTextLabel {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

        font: ui.theme.largeBodyBoldFont

        text: repeaterModel.stringName
    }

    RoundedRadioButton {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: 5
        checked: checkPedalState(repeaterModel.stringId, 0)

        navigation.name: repeaterModel.stringName + " Flat"
        navigation.panel: navPanel
        navigation.order: repeaterModel.stringId * 3 + 1
        navigation.accessible.name: repeaterModel.stringName + qsTrc("notation", " flat")

        onToggled: updatePedalState(repeaterModel.stringId, 0)
    }

    RoundedRadioButton {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: 5
        checked: checkPedalState(repeaterModel.stringId, 1)

        navigation.name: repeaterModel.stringName + " Natural"
        navigation.panel: navPanel
        navigation.order: repeaterModel.stringId * 3 + 2
        navigation.accessible.name: repeaterModel.stringName + qsTrc("notation", " natural")

        onToggled: updatePedalState(repeaterModel.stringId, 1)
    }

    RoundedRadioButton {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: 5
        checked: checkPedalState(repeaterModel.stringId, 2)

        navigation.name: repeaterModel.stringName + " Sharp"
        navigation.panel: navPanel
        navigation.order: repeaterModel.stringId * 3 + 3
        navigation.accessible.name: repeaterModel.stringName + qsTrc("notation", " sharp")

        onToggled: updatePedalState(repeaterModel.stringId, 2)
    }
}
