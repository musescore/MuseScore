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

    objectName: "VerticalFrameSettings"

    height: implicitHeight
    spacing: 12

    function focusOnFirst() {
        heightSection.focusOnFirst()
    }

    PropertyCheckBox {
        id: matchStaffSize

        navigation.name: "Scale with staff size"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRowStart + 1

        text: qsTrc("inspector", "Scale with staff size")
        propertyItem: root.model ? root.model.isSizeSpatiumDependent : null
    }

    SpinBoxPropertyView {
        id: heightSection
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 2

        titleText: qsTrc("inspector", "Height")
        propertyItem: root.model ? root.model.frameHeight : null

        icon: IconCode.VERTICAL

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 1
    }

    SeparatorLine { anchors.margins: -12 }

    VerticalGapsSection {
        id: verticalGapsSection
        gapAbove: root.model ? root.model.gapAbove : null
        gapBelow: root.model ? root.model.gapBelow : null
        notationGapAbove: root.model ? root.model.paddingToNotationAbove : null
        notationGapBelow: root.model ? root.model.paddingToNotationBelow : null

        navigationPanel: root.navigationPanel
        navigationRowStart: heightSection.navigationRowEnd + 1
    }

    SeparatorLine { anchors.margins: -12 }

    HorizontalMarginsSection {
        id: horizontalMarginsSection
        frameLeftMargin: root.model ? root.model.frameLeftMargin : null
        frameRightMargin: root.model ? root.model.frameRightMargin : null

        navigationPanel: root.navigationPanel
        navigationRowStart: verticalGapsSection.navigationRowEnd + 1
    }

    VerticalMarginsSection {
        frameTopMargin: root.model ? root.model.frameTopMargin : null
        frameBottomMargin: root.model ? root.model.frameBottomMargin : null

        navigationPanel: root.navigationPanel
        navigationRowStart: horizontalMarginsSection.navigationRowEnd + 1
    }
}
