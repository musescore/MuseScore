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
import QtQuick 2.9
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../../common"

InspectorPropertyView {
    id: root

    property QtObject widthProperty: undefined

    anchors.left: parent.left
    anchors.right: parent.horizontalCenter
    anchors.rightMargin: 2

    titleText: qsTrc("inspector", "Width")
    propertyItem: widthProperty

    IncrementalPropertyControl {
        icon: IconCode.HORIZONTAL

        isIndeterminate: widthProperty ? widthProperty.isUndefined : false
        currentValue: widthProperty ? widthProperty.value : 0

        onValueEdited: { widthProperty.value = newValue }
    }
}
