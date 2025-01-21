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

import Muse.UiComponents 1.0

import "../../internal"

BaseSection {
    id: root

    property bool dynamicsApplyToAllVoices: false

    signal dynamicsApplyToAllVoicesChangeRequested(bool value)

    title: qsTrc("appshell/preferences", "Voice assignment")

    StyledTextLabel {
        width: parent.width
        text: qsTrc("appshell/preferences", "When entered, dynamics and hairpins should affect:")
        horizontalAlignment: Text.AlignLeft
    }

    RoundedRadioButton {
        width: parent.width

        text: qsTrc("appshell/preferences", "All voices on the instrument")

        navigation.name: "AllVoicesOnInstrumentBox"
        navigation.panel: root.navigation
        navigation.row: 0

        checked: root.dynamicsApplyToAllVoices

        onToggled: {
            root.dynamicsApplyToAllVoicesChangeRequested(checked)
        }
    }

    RoundedRadioButton {
        width: parent.width

        text: qsTrc("appshell/preferences", "Only the voice they are applied to")

        navigation.name: "OnlyVoiceTheyAreAppliedToBox"
        navigation.panel: root.navigation
        navigation.row: 1

        checked: !root.dynamicsApplyToAllVoices

        onToggled: {
            root.dynamicsApplyToAllVoicesChangeRequested(!checked)
        }
    }
}
