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
import QtQuick 2.9
import QtQuick.Controls 2.2

import MuseScore.UiComponents 1.0

StyledPopup {
    id: root

    property var voicesVisibility: [] // array of bool
    signal voiceVisibilityChangeRequested(var voiceIndex, var voiceVisible)

    height: contentColumn.implicitHeight + bottomPadding + topPadding
    width: contentColumn.implicitWidth + leftPadding + rightPadding

    Column {
        id: contentColumn

        spacing: 18

        StyledTextLabel {
            text: qsTrc("notation", "Voices visible on this score")
        }

        ListView {
            spacing: 8

            height: contentHeight
            width: parent.width

            model: root.voicesVisibility

            delegate: CheckBox {
                checked: modelData
                text: qsTrc("notation", "Voice ") + (model.index + 1)

                onClicked: {
                    checked = !checked
                    root.voicesVisibility[model.index] = checked
                    root.voiceVisibilityChangeRequested(model.index, checked)
                }
            }
        }
    }
}
