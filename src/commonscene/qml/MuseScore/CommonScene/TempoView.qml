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

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Item {
    id: root

    property alias noteSymbol: noteSymbolLabel.text
    property int tempoValue: 0

    property alias noteSymbolFont: noteSymbolLabel.font
    property alias tempoValueFont: tempoValueLabel.font

    property alias noteSymbolTopPadding: noteSymbolLabel.topPadding

    implicitWidth: contentRow.implicitWidth
    implicitHeight: contentRow.implicitHeight

    RowLayout {
        id: contentRow

        anchors.centerIn: parent

        spacing: 0

        StyledTextLabel {
            id: noteSymbolLabel

            topPadding: 10
            lineHeightMode: Text.FixedHeight
            lineHeight: 10

            font.family: ui.theme.musicalFont.family
            font.pixelSize: ui.theme.musicalFont.pixelSize
            font.letterSpacing: 1
        }

        StyledTextLabel {
            id: tempoValueLabel

            text: " = " + root.tempoValue
        }
    }
}
