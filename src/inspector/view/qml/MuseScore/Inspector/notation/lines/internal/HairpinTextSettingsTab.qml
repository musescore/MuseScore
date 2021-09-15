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
import QtQuick.Controls 2.15

import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

import "../../../common"
import "../../../"

FocusableItem {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        InspectorPropertyView {
            titleText: qsTrc("inspector", "Beginning text")
            propertyItem: root.model ? root.model.beginingText : null

            TextInputField {
                isIndeterminate: root.model ? root.model.beginingText.isUndefined : false
                currentText: root.model ? root.model.beginingText.value : ""
                enabled: root.model ? root.model.beginingText.isEnabled : false

                onCurrentTextEdited: {
                    if (!root.model) {
                        return
                    }

                    root.model.beginingText.value = newTextValue
                }
            }
        }

        OffsetSection {
            horizontalOffset: root.model ? root.model.beginingTextHorizontalOffset : null
            verticalOffset: root.model ? root.model.beginingTextVerticalOffset : null
        }

        SeparatorLine { anchors.margins: -10 }

        InspectorPropertyView {
            titleText:  qsTrc("inspector", "Text when continuing to a new system")
            propertyItem: root.model ? root.model.continiousText : null

            TextInputField {
                isIndeterminate: root.model ? root.model.continiousText.isUndefined : false
                currentText: root.model ? root.model.continiousText.value : ""
                enabled: root.model ? root.model.continiousText.isEnabled : false

                onCurrentTextEdited: {
                    if (!root.model) {
                        return
                    }

                    root.model.continiousText.value = newTextValue
                }
            }
        }

        OffsetSection {
            horizontalOffset: root.model ? root.model.continiousTextHorizontalOffset : null
            verticalOffset: root.model ? root.model.continiousTextVerticalOffset : null
        }
    }
}
