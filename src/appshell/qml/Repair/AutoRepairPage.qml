/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
import MuseScore.Repair 1.0

import "internal"
import "../Preferences/internal"

RepairPage {
    id: root

    Component.onCompleted: {
        repairModel.load()
    }

    AutoRepairModel {
        id: repairModel

        onReceivingUpdateForCurrentLanguage: function(current, total, status) {
            languagesSection.setUpdateProgress(current, total, status)
        }
    }

    Column {
        width: parent.width
        spacing: root.sectionsSpacing

        LanguagesSection {
            id: languagesSection

            languages: repairModel.languages
            currentLanguageCode: repairModel.currentLanguageCode
            isNeedRestart: repairModel.isNeedRestart

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 1

            onLanguageSelected: function(languageCode) {
                repairModel.currentLanguageCode = languageCode
            }

            onCheckForUpdateRequested: {
                repairModel.checkUpdateForCurrentLanguage()
            }
        }

        SeparatorLine { }

        ProgramStartSection {
            startupModes: repairModel.startupModes
            scorePathFilter: repairModel.scorePathFilter()

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 2

            onCurrentStartupModesChanged: function(index) {
                repairModel.setCurrentStartupMode(index)
            }

            onStartupScorePathChanged: function(path) {
                repairModel.setStartupScorePath(path)
            }
        }

        /*
         * TODO: https://github.com/musescore/MuseScore/issues/9807
        KeyboardLayoutsSection {
            keyboardLayouts: repairModel.keyboardLayouts
            currentKeyboardLayout: repairModel.currentKeyboardLayout

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 2

            onKeyboardLayoutSelected: function(keyboardLayout) {
                repairModel.currentKeyboardLayout = keyboardLayout
            }
        }

        SeparatorLine { }
        */

        /*
         * TODO: https://github.com/musescore/MuseScore/issues/9807
        SeparatorLine { }

        RemoteControlSection {
            isOSCRemoteControl: repairModel.isOSCRemoteControl
            oscPort: repairModel.oscPort

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 4

            onRemoteControlChanged: function(control) {
                repairModel.isOSCRemoteControl = control
            }

            onPortChanged: function(port) {
                repairModel.oscPort = port
            }
        }*/
    }
}
