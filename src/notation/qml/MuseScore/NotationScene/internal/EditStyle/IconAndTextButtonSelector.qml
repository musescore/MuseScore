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
import QtQuick.Layouts 1.15

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0


StyleControlRowWithReset {
    id: root

    property alias model: radioButtonGroup.model

    RadioButtonGroup {
        id: radioButtonGroup

        anchors.fill: parent

        delegate: FlatRadioButton {
            height: 70

            navigation.accessible.name: modelData.title ? modelData.title : (modelData.text ? modelData.text : "")

            checked: styleItem.value === modelData.value
            onToggled: styleItem.value = modelData.value

            Column {
                anchors.centerIn: parent
                height: childrenRect.height
                spacing: 8

                StyledIconLabel {
                    anchors.horizontalCenter: parent.horizontalCenter
                    iconCode: modelData.iconCode ? modelData.iconCode : IconCode.NONE
                    font.pixelSize: 28
                }

                StyledTextLabel {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: modelData.text ? modelData.text : ""
                }
            }
        }
    }
}
