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
import QtQuick.Controls 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

import "internal"

PreferencesPage {
    id: root

    contentHeight: content.height

    Component.onCompleted: {
        preferencesModel.load()
    }

    GeneralPreferencesModel {
        id: preferencesModel
    }

    Column {
        id: content

        width: parent.width
        spacing: root.sectionsSpacing

        LanguagesSection {
            languages: preferencesModel.languages
            currentLanguageCode: preferencesModel.currentLanguageCode

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 1

            onLanguageSelected: function(languageCode) {
                preferencesModel.currentLanguageCode = languageCode
            }

            onUpdateTranslationsRequested: {
                root.hideRequested()
                preferencesModel.openUpdateTranslationsPage()
            }
        }

        SeparatorLine { }

        KeyboardLayoutsSection {
            keyboardLayouts: preferencesModel.keyboardLayouts
            currentKeyboardLayout: preferencesModel.currentKeyboardLayout

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 2

            onKeyboardLayoutSelected: function(keyboardLayout) {
                preferencesModel.currentKeyboardLayout = keyboardLayout
            }
        }

        SeparatorLine {
            visible: telemetrySection.visible
        }

        TelemetrySection {
            id: telemetrySection

            isTelemetryAllowed: preferencesModel.isTelemetryAllowed

            visible: false

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 3

            onTelemetryAllowedChanged: function(allowed) {
                preferencesModel.isTelemetryAllowed = allowed
            }
        }

        SeparatorLine { }

        AutoSaveSection {
            isAutoSaveEnabled: preferencesModel.isAutoSaveEnabled
            autoSaveInterval: preferencesModel.autoSaveInterval

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 4

            onAutoSaveEnabledChanged: function(enabled) {
                preferencesModel.isAutoSaveEnabled = enabled
            }

            onIntervalChanged: function(minutes) {
                preferencesModel.autoSaveInterval = minutes
            }
        }

        SeparatorLine { }

        RemoteControlSection {
            isOSCRemoteControl: preferencesModel.isOSCRemoteControl
            oscPort: preferencesModel.oscPort

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 5

            onRemoteControlChanged: function(control) {
                preferencesModel.isOSCRemoteControl = control
            }

            onPortChanged: function(port) {
                preferencesModel.oscPort = port
            }
        }
    }
}
