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
import QtQuick.Layouts 1.15

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

Rectangle {
    anchors.fill: parent
    color: ui.theme.backgroundPrimaryColor

    TupletCenteringSelectorModel {
        id: tupletCenteringSelectorModel
    }

    ColumnLayout {
        width: parent.width
        spacing: 7

        StyledTextLabel {
            horizontalAlignment: Text.AlignLeft
            text: qsTrc("notation/editstyle/tupletCentering", "Number alignment")
        }

        RadioButtonGroup {
            orientation: ListView.horizontal
            spacing: 8

            model: [
                {iconCode: IconCode.TUPLET_GRAPHICAL_CENTER, text: qsTrc("notation/editstyle/tupletCentering", "Graphical center"), value: false },
                {iconCode: IconCode.TUPLET_RYTHMIC_CENTER, text: qsTrc("notation/editstyle/tupletCentering", "Rhythmic center"), value: true },
            ]

            delegate: FlatRadioButton {
                width: 140
                height: 65

                Column {
                    anchors.centerIn: parent
                    spacing: 4

                    StyledIconLabel {
                        anchors.horizontalCenter: parent.horizontalCenter
                        iconCode: modelData.iconCode
                        font.pixelSize: 32
                    }

                    StyledTextLabel {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: modelData.text
                    }
                }

                checked: tupletCenteringSelectorModel.tupletNumberRythmicCenter.value === modelData.value
                onToggled: tupletCenteringSelectorModel.tupletNumberRythmicCenter.value = modelData.value
            }
        }
    }
}
