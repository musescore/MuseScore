/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
import QtQuick.Controls

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

ComboBox {
        // TODO - replace with StyledDropdown once this whole dialog is written in QML
        id: comboDropdown

        required property StyleItem styleItem

        signal handleItem(var value)

        width: 172

        textRole: "text"
        valueRole: "value"

        flat: true

        background: Rectangle {
            id: backgroundItem
            implicitWidth: 120
            implicitHeight: 30
            anchors.fill: parent
            border.width: ui.theme.borderWidth
            border.color: ui.theme.strokeColor
            color: ui.theme.buttonColor
            radius: 3
            opacity: 0.7
        }

        indicator: StyledIconLabel {
            id: dropIconItem
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 8

            iconCode: IconCode.SMALL_ARROW_DOWN
        }

        contentItem: StyledTextLabel {
            id: labelItem
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: dropIconItem.left
            anchors.leftMargin: 12
            anchors.rightMargin: 6
            horizontalAlignment: Text.AlignLeft
            text: comboDropdown.displayText
        }

        delegate: ListItemBlank {
            id: delegate

            objectName: "dropitem"

            height: comboDropdown.itemHeight
            width: comboDropdown.contentWidth

            normalColor: ui.theme.buttonColor
            radius: 0

            isSelected: model.index === comboDropdown.currentIndex

            StyledTextLabel {
                id: label
                anchors.fill: parent
                anchors.leftMargin: 12
                horizontalAlignment: Text.AlignLeft

                text: Utils.getItemValue(comboDropdown.model, model.index, comboDropdown.textRole, "")
            }
            onClicked: {
                comboDropdown.activated(model.index)
                comboDropdown.popup.close()
            }
        }

        Component.onCompleted: function() {
            currentIndex = indexOfValue(styleItem.value)
        }

        onActivated: function(index) {
            currentIndex = index
            var value = Utils.getItemValue(comboDropdown.model, index, comboDropdown.valueRole, undefined)
            styleItem.value = value
            comboDropdown.handleItem(value)
        }
    }
