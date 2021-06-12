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
import MuseScore.Preferences 1.0

import "internal"

PreferencesPage {
    id: root

    contentHeight: content.height

    AccessibilityPreferencesModel {
        id: accessibilityModel
    }

    Column {
        id: content

        width: parent.width
        height: childrenRect.height

        spacing: 24

        readonly property int firstColumnWidth: 212

        ThemesSection {
            width: parent.width

            titleText: qsTrc("appshell", "High Contrast Themes")
            themes: accessibilityModel.highContrastThemes
            currentThemeIndex: accessibilityModel.currentThemeIndex

            onThemeChangeRequested: {
                accessibilityModel.currentThemeIndex = newThemeIndex
            }
        }

        SeparatorLine {}

        StyledTextLabel {
            text: qsTrc("appshell", "UI Colours")
            font: ui.theme.bodyBoldFont
        }

        Repeater {
            width: parent.width
            //spacing: 12
            //orientation: Qt.Vertical

            model: [
                { textRole: "Accent Color:", colorRole: ui.theme.accentColor},
                { textRole: "Text and Icons:", colorRole: ui.theme.fontPrimaryColor},
                { textRole: "Disabled Text:", colorRole: "#000000"},
                { textRole: "Stroke:", colorRole: ui.theme.strokeColor}
            ]

            delegate: Row {
                anchors.left: parent.left
                anchors.right: parent.right

                StyledTextLabel {
                    text: modelData["textRole"]
                    width: content.firstColumnWidth
                    horizontalAlignment: Text.AlignLeft
                }

                ColorPicker {
                    y: -8
                    width: 112
                    color: modelData["colorRole"]
                    overrideDefaultColors: true
                    setBorderColor: ui.theme.strokeColor
                }
            }
        }
    }
}
