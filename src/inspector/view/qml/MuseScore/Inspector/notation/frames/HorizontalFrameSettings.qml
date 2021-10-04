/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

import "../../common"
import "internal"

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowOffset: 1

    objectName: "HorizontalFrameSettings"

    height: implicitHeight
    spacing: 12

    SpinBoxPropertyView {
        id: widthSection
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 2

        titleText: qsTrc("inspector", "Width")
        propertyItem: root.model ? root.model.frameWidth : null

        icon: IconCode.HORIZONTAL

        navigation.panel: root.navigationPanel
        navigationRowStart: root.navigationRowOffset + 1
    }

    SeparatorLine { anchors.margins: -10 }

    HorizontalGapsSection {
        id: horizontalGapsSection
        leftGap: root.model ? root.model.leftGap : null
        rightGap: root.model ? root.model.rightGap: null

        navigationPanel: root.navigationPanel
        navigationRowStart: widthSection.navigationRowEnd + 1
    }

    SeparatorLine { anchors.margins: -10 }

    CheckBox {
        isIndeterminate: root.model ? root.model.shouldDisplayKeysAndBrackets.isUndefined : false
        checked: root.model && !isIndeterminate ? root.model.shouldDisplayKeysAndBrackets.value : false
        text: qsTrc("inspector", "Display key, brackets and braces")

        navigation.name: "DisplayKeysAndBracketsCheckBox"
        navigation.panel: root.navigationPanel
        navigation.row: horizontalGapsSection.navigationRowEnd + 1
        navigation.enabled: root.enabled

        onClicked: { root.model.shouldDisplayKeysAndBrackets.value = !checked }
    }
}
