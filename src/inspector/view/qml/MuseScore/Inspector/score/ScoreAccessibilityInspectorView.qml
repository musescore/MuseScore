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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.Inspector 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

import "../common"

InspectorSectionView {
    id: root

    implicitHeight: contentColumn.implicitHeight

    Column {
        id: contentColumn
        width: parent.width
        spacing: 8

        StyledTextLabel {
            width: parent.width
            text: qsTrc("inspector", "Score style preset")
            horizontalAlignment: Text.AlignLeft
        }

        StyledDropdown {
            id: scoreStylePreset
            width: parent.width

            model: root.model ? root.model.possibleStylePresets : null
            currentIndex: root.model ? root.model.currentStylePresetIndex : -1

            Component.onCompleted: {
                root.model.updateCurrentStylePreset()
            }

            onActivated: function(index, value) {
                root.model.currentStylePresetIndex = index
            }
        }
    }
}
