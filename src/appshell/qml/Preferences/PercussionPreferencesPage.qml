/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
import MuseScore.Preferences 1.0

import "internal"

PreferencesPage {
    id: root

    PercussionPreferencesModel {
        id: percussionPreferencesModel
    }

    Component.onCompleted: {
        percussionPreferencesModel.init()
    }

    BaseSection {
        id: percussionPanelPreferences

        title: qsTrc("appshell/preferences", "Percussion")
        rowSpacing: 20

        navigation.section: root.navigationSection

        CheckBox {
            id: unpitchedSelectedCheckbox

            enabled: percussionPreferencesModel.useNewPercussionPanel
            width: parent.width

            text: qsTrc("appshell/preferences", "Open the percussion panel when an unpitched staff is selected")

            navigation.name: "UnpitchedSelectedCheckbox"
            navigation.panel: percussionPanelPreferences.navigation
            navigation.row: 0

            checked: percussionPreferencesModel.autoShowPercussionPanel

            onClicked:  {
                percussionPreferencesModel.autoShowPercussionPanel = !unpitchedSelectedCheckbox.checked
            }
        }

        //! NOTE: "Pad swap options" and the associated dialog were dropped from percussion panel MVP (version 4.5).
        //! See PR #25810 when re-implementing...
        // Column {
        //     id: swappingOptionsColumn
        //     ...
        // }

        SeparatorLine {}

        Row {
            id: useLegacyToggleRow

            height: useLegacyToggle.height
            width: parent.width

            spacing: 6

            ToggleButton {
                id: useLegacyToggle

                checked: !percussionPreferencesModel.useNewPercussionPanel

                navigation.name: "UseLegacyPercussionPanel"
                navigation.panel: percussionPanelPreferences.navigation
                navigation.row: alwaysAsk.navigation.row + 1

                onToggled: {
                    percussionPreferencesModel.useNewPercussionPanel = !percussionPreferencesModel.useNewPercussionPanel
                }
            }

            StyledTextLabel {
                id: legacyToggleInfo

                height: parent.height

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                wrapMode: Text.Wrap
                text: qsTrc("notation/percussion", "Use legacy percussion panel")
            }
        }
    }
}
