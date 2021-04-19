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

Column {
    id: root

    property alias currentThemeIndex: view.currentIndex
    property alias themes: view.model

    signal themeChangeRequested(var newThemeIndex)

    spacing: 18

    StyledTextLabel {
        text: qsTrc("appshell", "Themes")
        font: ui.theme.bodyBoldFont
    }

    ListView {
        id: view

        width: parent.width
        height: contentHeight
        contentHeight: 120

        orientation: Qt.Horizontal
        interactive: false

        spacing: 106

        delegate: Column {
            width: 112
            height: 120

            spacing: 16

            ThemeSample {
                strokeColor: modelData.strokeColor
                backgroundPrimaryColor: modelData.backgroundPrimaryColor
                backgroundSecondaryColor: modelData.backgroundSecondaryColor
                fontPrimaryColor: modelData.fontPrimaryColor
                buttonColor: modelData.buttonColor
                accentColor: modelData.accentColor

                onClicked: {
                    root.themeChangeRequested(model.index)
                }
            }

            RoundedRadioButton {
                width: parent.width

                checked: view.currentIndex === model.index
                text: modelData.title

                onClicked: {
                    root.themeChangeRequested(model.index)
                }
            }
        }
    }
}
