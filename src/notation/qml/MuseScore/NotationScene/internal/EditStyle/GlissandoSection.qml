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
import QtQuick.Controls 2.15

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

Rectangle {
    id: root
    anchors.fill: parent
    color: ui.theme.backgroundPrimaryColor

    GlissandoSectionModel {
        id: glissandoSectionModel
    }

    ColumnLayout {
        width: parent.width
        spacing: 12
        Layout.minimumWidth: 120

        RadioButtonSelectorWithReset {
            styleItem: glissandoSectionModel.glissandoLineType
            label: qsTrc("notation", "Line type:")

            model: [
                { text: qsTrc("notation", "Straight"), value: 0 },
                { text: qsTrc("notation", "Wavy"), value: 1}
            ]
        }

        LineStyleSection {
            id: lineStyleSection

            lineStyle: glissandoSectionModel ? glissandoSectionModel.glissandoLineStyle : null
            dashLineLength: glissandoSectionModel ? glissandoSectionModel.glissandoLineStyleDashSize : null
            dashGapLength: glissandoSectionModel ? glissandoSectionModel.glissandoLineStyleGapSize : null
            lineWidth: glissandoSectionModel ? glissandoSectionModel.glissandoLineWidth : null
        }

        RowLayout {
            id: textRow
            spacing: 6
            CheckBox {
                id: showTextCheckBox
                width: parent.width

                checked: glissandoSectionModel.glissandoShowText && Boolean(glissandoSectionModel.glissandoShowText.value)
                onClicked: {
                    if (glissandoSectionModel.glissandoShowText) {
                        glissandoSectionModel.glissandoShowText.value = !checked
                    }
                }
            }

            StyledTextLabel {
                Layout.minimumWidth: 120 - textRow.spacing - showTextCheckBox.width
                horizontalAlignment: Text.AlignLeft
                text: qsTrc("notation", "Show text:")
            }

            TextInputField {
                id: textField
                Layout.preferredWidth: 80
                Layout.fillWidth: false
                currentText: glissandoSectionModel.glissandoText ? glissandoSectionModel.glissandoText.value : ""
                enabled: glissandoSectionModel.glissandoShowText

                onTextEditingFinished: function(newTextValue) {
                    if (glissandoSectionModel.glissandoShowText) {
                        glissandoSectionModel.glissandoText.value = newTextValue
                    }
                }
            }

            Item {
                Layout.fillWidth: true
            }

            FlatButton {
                icon: IconCode.UNDO
                enabled: !glissandoSectionModel.glissandoText.isDefault
                onClicked: glissandoSectionModel.glissandoText.value = glissandoSectionModel.glissandoText.defaultValue
            }
        }
    }
}
