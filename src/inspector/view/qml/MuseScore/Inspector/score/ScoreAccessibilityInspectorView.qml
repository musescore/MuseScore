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

        DropdownPropertyView {
            id: scoreStylePreset

            titleText: qsTrc("inspector", "Score style preset")
            propertyItem: root.model ? root.model.scoreStylePreset : null
            visible: true
            enabled: true

            model: root.model ? root.model.possibleScoreStylePreset() : []

            Connections {
                target: root.model
                onScoreStylePresetChanged: {
                    scoreStylePreset.model = root.model ? root.model.possibleScoreStylePreset() : [];
                }
            }
        }
    }
}
