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

    ThemesPageModel {
        id: model
    }

    Component.onCompleted: {
        model.load()
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 32

        ThemeSamplesList {
            Layout.alignment: Qt.AlignHCenter

            themes: model.highContrastEnabled ? model.highContrastThemes : model.generalThemes
            currentThemeCode: model.currentThemeCode

            spacing: 64

            onThemeChangeRequested: function(newThemeCode) {
                model.currentThemeCode = newThemeCode
            }
        }

        CheckBox {
            Layout.alignment: Qt.AlignHCenter
            width: implicitWidth

            text: qsTrc("appshell", "Enable high contrast")
            checked: model.highContrastEnabled
            onClicked: {
                model.highContrastEnabled = !checked
            }
        }
    }
}
