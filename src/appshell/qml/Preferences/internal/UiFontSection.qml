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

    property alias allFonts: selectFontControl.model

    property alias currentFontIndex: selectFontControl.currentIndex
    property alias bodyTextSize: bodyTextSizeControl.currentValue

    property int firstColumnWidth: 0

    signal fontChangeRequested(var newFontIndex)
    signal bodyTextSizeChangeRequested(var newBodyTextSize)

    spacing: 18

    StyledTextLabel {
        text: qsTrc("appshell", "Appearance")
        font: ui.theme.bodyBoldFont
    }

    Column {
        spacing: 8

        ComboBoxWithTitle {
            id: selectFontControl

            title: qsTrc("appshell", "Font face:")
            titleWidth: root.firstColumnWidth

            onValueEdited: {
                root.fontChangeRequested(currentIndex)
            }
        }

        IncrementalPropertyControlWithTitle {
            id: bodyTextSizeControl

            title: qsTrc("appshell", "Body text size:")
            titleWidth: root.firstColumnWidth

            minValue: 8
            maxValue: 24
            measureUnitsSymbol: qsTrc("appshell", "pt")

            onValueEdited: {
                root.bodyTextSizeChangeRequested(newValue)
            }
        }
    }
}
