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

    objectName: "TremoloSettings"

    spacing: 12

    function focusOnFirst() {
        styleSection.focusOnFirst()
    }

    FlatRadioButtonGroupPropertyView {
        id: styleSection
        titleText: qsTrc("inspector", "Style (between notes)")
        propertyItem: root.model ? root.model.style : null

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 1

        model: [
            { iconCode: IconCode.TREMOLO_STYLE_DEFAULT, value: TremoloTypes.STYLE_DEFAULT, title: qsTrc("inspector", "Default") },
            { iconCode: IconCode.TREMOLO_STYLE_TRADITIONAL, value: TremoloTypes.STYLE_TRADITIONAL, title: qsTrc("inspector", "Traditional") },
            { iconCode: IconCode.TREMOLO_STYLE_TRADITIONAL_ALTERNATE, value: TremoloTypes.STYLE_TRADITIONAL_ALTERNATE, title: qsTrc("inspector", "Traditional alternative") }
        ]
    }

    DirectionSection {
        id: directionSection

        titleText: qsTrc("inspector", "Stem direction")
        propertyItem: root.model ? root.model.direction : null

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 2
    }
}
