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

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "BracketSettings"

    spacing: 12

    function focusOnFirst() {
        columnSection.focusOnFirst()
    }

    Item {
        height: childrenRect.height
        width: parent.width

        enabled: root.model ? root.model.areSettingsAvailable : false

        SpinBoxPropertyView {
            id: columnSection
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            titleText: qsTrc("inspector", "Column")
            propertyItem: root.model ? root.model.bracketColumnPosition : null
            showButton: false

            step: 1
            decimals: 0
            maxValue: root.model ? root.model.maxBracketColumnPosition : 0
            minValue: 0

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 1
        }

        SpinBoxPropertyView {
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            titleText: qsTrc("inspector", "Span")
            propertyItem: root.model ? root.model.bracketSpanStaves : null
            showButton: false

            step: 1
            decimals: 0
            maxValue: root.model ? root.model.maxBracketSpanStaves : 0
            minValue: 1

            navigationPanel: root.navigationPanel
            navigationRowStart: columnSection.navigationRowEnd + 1
        }
    }

    StyledTextLabel {
        width: parent.width
        visible: root.model ? !root.model.areSettingsAvailable : false
        text: qsTrc("inspector", "You have multiple brackets selected. Select a single bracket to edit its settings.")
        wrapMode: Text.Wrap
    }
}
