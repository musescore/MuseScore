/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
import QtQuick
import QtQuick.Controls

import MuseScore.Inspector
import Muse.UiComponents
import Muse.Ui

import "../beams"
import "../../common"

FocusableItem {
    id: root

    property QtObject model: null
    readonly property QtObject beamModesModel: model ? model.beamModesModel : null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    implicitWidth: beamTypeSection.implicitWidth
    implicitHeight: beamTypeSection.implicitHeight

    BeamTypeSelector {
        id: beamTypeSection
        titleText: qsTrc("inspector", "Beam type")
        propertyItem: root.beamModesModel ? root.beamModesModel.mode : null
        enabled: root.beamModesModel && !root.beamModesModel.isEmpty

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart
    }
}
