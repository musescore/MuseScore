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
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

BaseSection {
    id: root

    title: qsTrc("appshell", "UI colors")
    navigation.direction: NavigationPanel.Both

    property bool scoreInversionChecked: false

    signal scoreInversionRequested(var newValue)
    signal colorChangeRequested(var newColor, var propertyType)

    CheckBox {
        id: scoreInversionEnable

        checked: root.scoreInversionChecked
        text: qsTrc("appshell", "Invert score colors")

        onClicked: {
            root.scoreInversionRequested(!checked)
        }
    }

    GridLayout {
        id: grid

        columnSpacing: parent.width / 8
        rowSpacing: 20
        columns: 2

        Repeater {

            model: [
                { textRole: "Accent color:", colorRole: ui.theme.accentColor, typeRole: AppearancePreferencesModel.AccentColor},
                { textRole: "Text and icons:", colorRole: ui.theme.fontPrimaryColor, typeRole: AppearancePreferencesModel.TextAndIconsColor},
                { textRole: "Disabled text:", colorRole: "#000000", typeRole: AppearancePreferencesModel.DisabledColor},
                { textRole: "Border color:", colorRole: ui.theme.strokeColor, typeRole: AppearancePreferencesModel.BorderColor}
            ]

            delegate: Row {

                StyledTextLabel {
                    id: titleLabel
                    anchors.verticalCenter: parent.verticalCenter
                    text: modelData["textRole"]
                    width: root.columnWidth / 2
                    horizontalAlignment: Text.AlignLeft
                }

                ColorPicker {
                    width: 112
                    color: modelData["colorRole"]

                    navigation.name: titleLabel.text
                    navigation.panel: root.navigation
                    navigation.row: index / grid.columns
                    navigation.column: index % grid.columns
                    navigation.accessible.name: titleLabel.text

                    onNewColorSelected: {
                        root.colorChangeRequested(newColor, modelData.typeRole)
                    }
                }
            }
        }
    }
}
