/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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

pragma ComponentBehavior: Bound

import QtQuick

import Muse.Ui
import Muse.UiComponents
import MuseScore.NotationScene

StyleControlRowWithReset {
    id: root

    property alias model: radioButtonGroup.model

    property double buttonHeight: 70

    RadioButtonGroup {
        id: radioButtonGroup

        anchors.fill: parent

        delegate: FlatRadioButton {
            id: delegateButton

            required property var modelData
            required property int index

            height: root.buttonHeight

            navigation.accessible.name: modelData.title || modelData.text || ""

            checked: root.styleItem.value === modelData.value
            onToggled: root.styleItem.value = modelData.value

            Column {
                anchors.centerIn: parent
                spacing: 8

                StyledIconLabel {
                    anchors.horizontalCenter: parent.horizontalCenter
                    iconCode: delegateButton.modelData.iconCode ?? IconCode.NONE
                    font.pixelSize: delegateButton.modelData.iconSize ?? 28
                }

                StyledTextLabel {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: delegateButton.modelData.text ?? ""
                }
            }
        }
    }
}
