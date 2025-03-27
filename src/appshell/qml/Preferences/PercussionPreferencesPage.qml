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

    Column {
        id: contentColumn

        width: root.width
        spacing: root.sectionsSpacing

        BaseSection {
            id: autoShowSection

            enabled: percussionPreferencesModel.useNewPercussionPanel

            width: parent.width
            rowSpacing: 16

            title: qsTrc("notation/percussion", "Triggering the percussion panel")

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 1

            StyledTextLabel {
                id: autoShowLabel

                width: parent.width

                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.Wrap
                text: qsTrc("notation/percussion", "Open the panel automatically:")
            }

            RadioButtonGroup {
                id: autoShowModesBox

                property int navigationRowEnd: model.length

                spacing: 12
                orientation: Qt.Vertical

                width: parent.width

                model: percussionPreferencesModel.autoShowModes

                delegate: RoundedRadioButton {
                    width: ListView.view?.width ?? 0

                    text: modelData.title
                    checked: modelData.value === percussionPreferencesModel.autoShowMode

                    navigation.name: modelData.title
                    navigation.panel: autoShowSection.navigation
                    navigation.row: model.index
                    navigation.column: 0

                    onToggled: {
                        percussionPreferencesModel.autoShowMode = modelData.value
                    }
                }
            }

            CheckBox {
                id: autoCloseCheckBox

                enabled: !percussionPreferencesModel.neverAutoShow

                width: parent.width

                text: qsTrc("notation/percussion", "Close the panel automatically")

                navigation.name: "AutoClosePercussionPanelCheckBox"
                navigation.panel: autoShowSection.navigation
                navigation.row: autoShowModesBox.navigationRowEnd
                navigation.column: 0

                checked: !percussionPreferencesModel.neverAutoShow && percussionPreferencesModel.autoClosePercussionPanel

                onClicked:  {
                    percussionPreferencesModel.autoClosePercussionPanel = !autoCloseCheckBox.checked
                }
            }
        }

        SeparatorLine {}

        //! NOTE: "Pad swap options" and the associated dialog were dropped from percussion panel MVP (version 4.5).
        //! See PR #25810 when re-implementing...
        // BaseSection {
        //     id: padSwappingOptionsSection
        //     ...
        // }

        // SeparatorLine {}

        BaseSection {
            id: legacyToggleSection

            width: parent.width
            rowSpacing: 20

            title: qsTrc("appshell/preferences", "Legacy panel")

            navigation.section: root.navigationSection
            navigation.order: autoShowSection.navigation.order + 1

            Row {
                id: useLegacyToggleRow

                height: useLegacyToggle.height
                width: parent.width

                spacing: 6

                ToggleButton {
                    id: useLegacyToggle

                    checked: !percussionPreferencesModel.useNewPercussionPanel

                    navigation.name: "UseLegacyPercussionPanel"
                    navigation.panel: legacyToggleSection.navigation

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
}
