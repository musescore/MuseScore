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
import QtQuick.Controls 2.2
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import "../../common"

StyledPopup {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        InspectorPropertyView {
            titleText: qsTrc("inspector", "Jump to")
            propertyItem: model ? model.jumpTo : null

            TextInputField {
                isIndeterminate: model ? model.jumpTo.isUndefined : false
                currentText: model ? model.jumpTo.value : ""
                enabled: model ? model.jumpTo.isEnabled : false

                onCurrentTextEdited: {
                    if (!root.model) {
                        return
                    }

                    root.model.jumpTo.value = newTextValue
                }
            }
        }

        InspectorPropertyView {

            titleText: qsTrc("inspector", "Play until")
            propertyItem: model ? model.playUntil : null

            TextInputField {
                isIndeterminate: model ? model.playUntil.isUndefined : false
                currentText: model ? model.playUntil.value : ""
                enabled: model ? model.playUntil.isEnabled : false

                onCurrentTextEdited: {
                    if (!root.model) {
                        return
                    }

                    root.model.playUntil.value = newTextValue
                }
            }
        }

        InspectorPropertyView {
            titleText: qsTrc("inspector", "Continue at")
            propertyItem: model ? model.continueAt : null

            TextInputField {
                isIndeterminate: model ? model.continueAt.isUndefined : false
                currentText: model ? model.continueAt.value : ""
                enabled: model ? model.continueAt.isEnabled : false
            }
        }

        CheckBox {
            isIndeterminate: model ? model.hasToPlayRepeats.isUndefined : false
            checked: model && !isIndeterminate ? model.hasToPlayRepeats.value : false
            text: qsTrc("inspector", "Play repeats")

            onClicked: { model.hasToPlayRepeats.value = !checked }
        }
    }
}

