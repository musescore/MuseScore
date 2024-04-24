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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../common"

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "ArticulationSettings"

    spacing: 12

    function focusOnFirst() {
        placementSection.focusOnFirst()
    }

    PlacementSection {
        id: placementSection

        titleText: qsTrc("inspector", "Placement")
        propertyItem: root.model ? root.model.placement : null

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart

        model: [
            { text: qsTrc("inspector", "Auto"), value: ArticulationTypes.TYPE_AUTO },
            { text: qsTrc("inspector", "Above"), value: ArticulationTypes.TYPE_TOP },
            { text: qsTrc("inspector", "Below"), value: ArticulationTypes.TYPE_BOTTOM }
        ]
    }

    FlatRadioButtonGroupPropertyView {
        id: articStemHALignSection

        titleText: qsTrc("inspector", "Articulation alignment when positioned at the stem")
        propertyItem: root.model ? root.model.articStemHAlign : null
        visible: root.model ? root.model.isArticStemHAlignAvailable : false

        navigationPanel: root.navigationPanel
        navigationRowStart: placementSection.navigationRowEnd + 1

        model: [
            { text: qsTrc("inspector", "Align with stem"), value: ArticulationTypes.STEM },
            { text: qsTrc("inspector", "Align with notehead"), value: ArticulationTypes.NOTEHEAD },
            { text: qsTrc("inspector", "Center between stem and notehead"), value: ArticulationTypes.AVERAGE }
        ]
    }
}
