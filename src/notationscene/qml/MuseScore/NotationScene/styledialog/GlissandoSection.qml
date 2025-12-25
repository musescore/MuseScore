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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import Muse.Ui
import Muse.UiComponents
import MuseScore.NotationScene

Rectangle {
    id: root
    anchors.fill: parent
    color: ui.theme.backgroundPrimaryColor

    GlissandoSectionModel {
        id: glissandoSectionModel
    }

    signal goToTextStylePage(string s)

    ColumnLayout {
        width: parent.width
        spacing: 12

        RadioButtonSelectorWithReset {
            styleItem: glissandoSectionModel.glissandoLineType
            label: qsTrc("notation", "Line type:")

            model: [
                { text: qsTrc("notation", "Straight"), value: 0 },
                { text: qsTrc("notation", "Wavy"), value: 1 }
            ]
        }

        LineStyleSection {
            id: lineStyleSection

            lineStyle: glissandoSectionModel.glissandoLineStyle
            dashLineLength: glissandoSectionModel.glissandoLineStyleDashSize
            dashGapLength: glissandoSectionModel.glissandoLineStyleGapSize
            lineWidth: glissandoSectionModel.glissandoLineWidth
        }

        TextFieldWithReset {
            id: textRow
            styleItem: glissandoSectionModel.glissandoText

            labelComponent: CheckBox {
                id: showTextCheckBox
                text: qsTrc("notation", "Show text:")

                checked: Boolean(glissandoSectionModel.glissandoShowText.value)
                onClicked: {
                    glissandoSectionModel.glissandoShowText.value = !checked
                }
            }
        }

        FlatButton {
            text: qsTrc("notation", "Edit glissando text style")

            onClicked: {
                root.goToTextStylePage("glissando")
            }
        }
    }
}
