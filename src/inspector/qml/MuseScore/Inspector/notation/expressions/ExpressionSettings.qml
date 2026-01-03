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
import QtQuick

import Muse.Ui
import Muse.UiComponents
import MuseScore.Inspector

import "../../common"

Column {
    id: root

    required property ExpressionSettingsModel model

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "ExpressionSettings"

    spacing: 12

    function focusOnFirst() {
        snapExpression.requestActiveFocus()
    }

    CheckBoxPropertyView {
        id: snapExpression

        navigationName: "Snap expression"
        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart

        titleText: qsTrc("inspector", "Align with preceding dynamic")
        propertyItem: root.model?.snapExpression ?? null
    }
}
