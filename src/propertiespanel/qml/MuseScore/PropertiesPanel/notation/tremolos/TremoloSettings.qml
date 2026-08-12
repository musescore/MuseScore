/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

import Muse.Ui
import Muse.UiComponents
import MuseScore.PropertiesPanel

import "../../common"

Column {
    id: root

    required property TremoloSettingsModel model

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "TremoloSettings"

    spacing: 12

    function focusOnFirst() {
        styleSection.focusOnFirst()
    }

    FlatRadioButtonGroupPropertyView {
        id: styleSection
        titleText: qsTrc("propertiespanel", "Style (between notes)")
        propertyItem: root.model ? root.model.style : null

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 1

        model: [
            { iconCode: IconCode.TREMOLO_STYLE_DEFAULT, value: TremoloTypes.STYLE_DEFAULT, title: qsTrc("propertiespanel", "Default") },
            { iconCode: IconCode.TREMOLO_STYLE_TRADITIONAL, value: TremoloTypes.STYLE_TRADITIONAL, title: qsTrc("propertiespanel", "Traditional") },
            { iconCode: IconCode.TREMOLO_STYLE_TRADITIONAL_ALTERNATE, value: TremoloTypes.STYLE_TRADITIONAL_ALTERNATE, title: qsTrc("propertiespanel", "Traditional alternative") }
        ]
    }

    DirectionSection {
        id: directionSection

        titleText: qsTrc("propertiespanel", "Stem direction")
        propertyItem: root.model ? root.model.direction : null

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 2
    }
}
