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

        Text {
            text: qsTr("Score style preset")
            color: ui.theme.fontPrimaryColor
        }

        StyledDropdown {
            id: scoreStylePreset
            width: parent.width
            model: []

            Component.onCompleted: {
                root.model.readStyleFileAccessibilityStyleEdited()
                updateModel()
                scoreStylePreset.currentIndex = root.model.scoreStylePreset()
            }

            onActivated: function(index, value) {
                scoreStylePreset.currentIndex = index
                root.model.loadStyle(value)
            }

            function updateModel() {
                const presets = root.model ? root.model.possibleScoreStylePreset() : []
                const extendedModel = []
                for (var i = 0; i < presets.length; i++) {
                    extendedModel.push(presets[i])
                    if (presets[i].edited) {
                        extendedModel.push({ text: presets[i].text + " (edited)", value: presets[i].value })
                    }
                }
                scoreStylePreset.model = extendedModel
            }
        }
    }

    Connections {
        target: root.model
        onAccessibilityStyleEditedChanged: {
            scoreStylePreset.updateModel()
            scoreStylePreset.currentIndex = root.model.scoreStylePreset()
        }
    }
}
