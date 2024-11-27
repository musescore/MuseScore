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
import "internal"

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "GlissandoSettings"

    spacing: 12

    function focusOnFirst() {
        lineSection.focusOnFirst()
    }

    FlatRadioButtonGroupPropertyView {
        id: lineSection
        titleText: qsTrc("inspector", "Line type")
        propertyItem: root.model ? root.model.lineType : null
        model: root.model ? root.model.possibleLineTypes() : null

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart
    }

    LineStyleSection {
        id: lineStyleSection

        thickness: root.model ? root.model.thickness : null

        lineStyle: root.model ? root.model.lineStyle : null
        dashLineLength: root.model ? root.model.dashLineLength : null
        dashGapLength: root.model ? root.model.dashGapLength : null

        navigationPanel: root.navigationPanel
        navigationRowStart: lineSection.navigationRowEnd + 1
    }

    SeparatorLine { anchors.margins: -12; visible: hooksSection.visible }

    PropertyCheckBox {
        id: showTextCheckBox
        text: qsTrc("inspector", "Show text")
        propertyItem: root.model ? root.model.showText : null

        navigation.name: "ShowTextCheckBox"
        navigation.panel: root.navigationPanel
        navigation.row: lineStyleSection.navigationRowEnd + 1
    }

    TextSection {
        titleText: qsTrc("inspector", "Text")
        propertyItem: root.model ? root.model.text : null

        navigationPanel: root.navigationPanel
        navigationRowStart: showTextCheckBox.navigation.row + 1
    }
}
