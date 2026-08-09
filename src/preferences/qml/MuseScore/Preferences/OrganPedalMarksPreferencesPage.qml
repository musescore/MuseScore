/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
import MuseScore.Preferences

import "internal"

PreferencesPage {
    id: root

    OrganPedalMarksPreferencesModel {
        id: organPedalMarksPreferencesModel
    }

    Component.onCompleted: {
        organPedalMarksPreferencesModel.load()
    }

    Column {
        id: contentColumn

        width: parent.width
        spacing: root.sectionsSpacing

        BaseSection {
            id: placementSection

            width: parent.width
            rowSpacing: 16

            title: qsTrc("notation/organpedalmarks", "Default pedal mark placement")

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 1

            RadioButtonGroup {
                id: placement

                property int navigationRowEnd: model.length

                spacing: 12
                orientation: Qt.Vertical

                width: parent.width

                model: organPedalMarksPreferencesModel.placements

                delegate: RoundedRadioButton {
                    required property var modelData
                    required property int index

                    width: ListView.view?.width ?? 0

                    text: modelData.title
                    checked: modelData.value === organPedalMarksPreferencesModel.placement

                    navigation.name: modelData.title
                    navigation.panel: placementSection.navigation
                    navigation.row: index
                    navigation.column: 0

                    onToggled: {
                        organPedalMarksPreferencesModel.placement = modelData.value
                    }
                }
            }
        }

        SeparatorLine { }

        BaseSection {
            id: popupSetSection

            width: parent.width
            rowSpacing: 16

            title: qsTrc("notation/organpedalmarks", "Pedal marks popup set")

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 2

            RadioButtonGroup {
                id: popupSet

                property int navigationRowEnd: model.length

                spacing: 12
                orientation: Qt.Vertical

                width: parent.width

                model: organPedalMarksPreferencesModel.popupSets

                delegate: RoundedRadioButton {
                    required property var modelData
                    required property int index

                    width: ListView.view?.width ?? 0

                    text: modelData.title
                    checked: modelData.value === organPedalMarksPreferencesModel.popupSet

                    navigation.name: modelData.title
                    navigation.panel: popupSetSection.navigation
                    navigation.row: index
                    navigation.column: 0

                    onToggled: {
                        organPedalMarksPreferencesModel.popupSet = modelData.value
                    }
                }
            }
        }

        SeparatorLine { }

        BaseSection {
            id: navigationPlacementSection

            width: parent.width
            rowSpacing: 16

            title: qsTrc("notation/organpedalmarks", "Navigation pedal mark placement")

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 3

            RadioButtonGroup {
                id: navigationPlacement

                property int navigationRowEnd: model.length

                spacing: 12
                orientation: Qt.Vertical

                width: parent.width

                model: organPedalMarksPreferencesModel.navigationPlacements

                delegate: RoundedRadioButton {
                    required property var modelData
                    required property int index

                    width: ListView.view?.width ?? 0

                    text: modelData.title
                    checked: modelData.value === organPedalMarksPreferencesModel.navigationPlacement

                    navigation.name: modelData.title
                    navigation.panel: navigationPlacementSection.navigation
                    navigation.row: index
                    navigation.column: 0

                    onToggled: {
                        organPedalMarksPreferencesModel.navigationPlacement = modelData.value
                    }
                }
            }
        }

        SeparatorLine { }

        BaseSection {
            id: defaultPedalMarkSection

            width: parent.width
            rowSpacing: 16

            title: qsTrc("notation/organpedalmarks", "Navigation default pedal mark")

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 4

            RadioButtonGroup {
                id: defaultPedalMark

                property int navigationRowEnd: model.length

                spacing: 12
                orientation: Qt.Vertical

                width: parent.width

                model: organPedalMarksPreferencesModel.pedalMarks

                // FlatRadioButtonList in the future
                delegate: RoundedRadioButton {
                    required property var modelData
                    required property int index

                    width: ListView.view?.width ?? 0

                    text: modelData.title
                    checked: modelData.value === organPedalMarksPreferencesModel.defaultPedalMark

                    navigation.name: modelData.title
                    navigation.panel: defaultPedalMarkSection.navigation
                    navigation.row: index
                    navigation.column: 0

                    onToggled: {
                        organPedalMarksPreferencesModel.defaultPedalMark = modelData.value
                    }
                }
            }
        }
    }
}
