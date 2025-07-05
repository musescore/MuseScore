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

    objectName: "ExpressionsSettings"

    spacing: 12

    function focusOnFirst() {
        snapExpression.navigation.requestActive()
    }

    InspectorPropertyView {
        id: layoutSection
        titleText: ""
        propertyItem: root.model ? root.model.snapExpression : null

        navigationName: "LayoutMenu"
        navigationPanel: root.navigationPanel
        navigationRowStart: size.navigationRowEnd + 1
        navigationRowEnd: verticalAlignmentButtonList.navigationRowEnd

        isModified: root.model ? root.model.snapExpression.isModified : false

        onRequestResetToDefault: {
            if (root.model) {
                root.model.snapExpression.resetToDefault()
            }
        }

        onRequestApplyToStyle: {
            if (root.model) {
                root.model.snapExpression.applyToStyle()
            }
        }

        Item {
            anchors.right: root.right
            anchors.left: root.left
            height: childrenRect.height

            PropertyCheckBox {
                id: snapExpression

                navigation.name: "SnapExpressionCheckBox"
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationRowStart

                text: qsTrc("inspector", "Align with preceding dynamic")
                propertyItem: root.model ? root.model.snapExpression : null
            }
        }
    }

    VoicesAndPositionSection {
        id: voicesAndPositionSection

        navigationPanel: root.navigationPanel
        navigationRowStart: layoutSection.navigationRowEnd + 1

        model: root.model
    }
}
