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

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Muse.UiComponents 1.0
import Muse.Ui 1.0
import MuseScore.Inspector 1.0

import "../../common"

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "RestSettings"

    width: parent.width
    spacing: 12

    readonly property QtObject beamModel: model ? model.modelByType(Inspector.TYPE_BEAM) : null

    function focusOnFirst() {
        restBeamSettings.navigation.requestActive()
    }

    RestBeamSettings {
        id: restBeamSettings
        
        height: implicitHeight

        model: root.beamModel

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 1000
    }
}
