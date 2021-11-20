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
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.AppShell 1.0

import "../shared"

Page {
    title: qsTrc("appshell", "Welcome to MuseScore 4")
    explanation: qsTrc("appshell", "Let's get started by choosing a theme")

    titleContentSpacing: 28

    ThemesPageModel {
        id: model
    }

    Component.onCompleted: {
        model.load()
    }

    ColumnLayout {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 28

        ThemeSamplesList {
            id: themeSamplesList
            Layout.alignment: Qt.AlignCenter

            themes: model.highContrastEnabled ? model.highContrastThemes : model.generalThemes
            currentThemeCode: model.currentThemeCode

            spacing: 64

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 1

            onThemeChangeRequested: function(newThemeCode) {
                model.currentThemeCode = newThemeCode
            }
        }

        AccentColorsList {
            id: accentColorsList
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: Math.max(implicitHeight, highContrastPreferencesHintLabel.implicitHeight)
            visible: !model.highContrastEnabled

            colors: model.accentColors
            currentColorIndex: model.currentAccentColorIndex

            sampleSize: 20
            spacing: 4

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 2

            onAccentColorChangeRequested: function(newColorIndex) {
                model.currentAccentColorIndex = newColorIndex
            }
        }

        StyledTextLabel {
            id: highContrastPreferencesHintLabel
            visible: model.highContrastEnabled
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(implicitHeight, accentColorsList.implicitHeight)
            text: qsTrc("appshell", "Further high contrast settings are available in Preferences.")
        }

        CheckBox {
            Layout.alignment: Qt.AlignCenter

            text: qsTrc("appshell", "Enable high contrast")
            checked: model.highContrastEnabled

            navigation.name: "EnableHighContrastCheckbox"
            navigation.order: 1
            navigation.panel: NavigationPanel {
                name: "EnableHighContrast"
                enabled: parent.enabled && parent.visible
                section: root.navigationSection
                order: 3
                direction: NavigationPanel.Horizontal
            }

            onClicked: {
                model.highContrastEnabled = !checked
            }
        }
    }
}
