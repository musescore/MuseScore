/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

        implicitWidth: 12 + implicitContentWidth + 6 + implicitIndicatorWidth + 8
        implicitContentWidthPolicy: ComboBox.WidestText

        textRole: "text"
        valueRole: "value"

        flat: true

        opacity: enabled ? 1.0 : ui.theme.itemOpacityDisabled

        background: Rectangle {
            id: backgroundItem
            implicitWidth: 120
            implicitHeight: 30
            anchors.fill: parent
            border.width: ui.theme.borderWidth
            border.color: ui.theme.strokeColor
            color: ui.theme.buttonColor
            radius: 3
            opacity: ui.theme.buttonOpacityNormal

            states: [
                State {
                    name: "hovered"
                    when: comboDropdown.hovered
                    PropertyChanges {
                        target: backgroundItem
                        opacity: ui.theme.buttonOpacityHover
                    }
                },
                State {
                    name: "pressed"
                    when: comboDropdown.pressed
                    PropertyChanges {
                        target: backgroundItem
                        opacity: ui.theme.buttonOpacityPressed
                    }
                }
            ]
        }

        indicator: StyledIconLabel {
            id: dropIconItem
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 8

            iconCode: IconCode.SMALL_ARROW_DOWN
        }

        // It must be a TextInput to make `implicitContentWidthPolicy` work
        contentItem: TextInput {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: dropIconItem.left
            anchors.leftMargin: 12
            anchors.rightMargin: 6

            readOnly: true
            enabled: false // don't consume mouse events

            color: ui.theme.fontPrimaryColor
            verticalAlignment: Text.AlignVCenter

            font {
                family: ui.theme.bodyFont.family
                pixelSize: ui.theme.bodyFont.pixelSize
            }

            text: comboDropdown.displayText
        }

        delegate: ListItemBlank {
            id: delegate

            objectName: "dropitem"

            height: comboDropdown.itemHeight
            width: comboDropdown.contentWidth

            normalColor: ui.theme.buttonColor

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
            if (!styleItem) {
                return;
            }

            currentIndex = indexOfValue(styleItem.value)

            styleItem.valueChanged.connect(function() {
                const idx = indexOfValue(styleItem.value);
                if (idx >= 0) {
                    currentIndex = idx;
                }
            })
        }

        onActivated: function(index) {
            currentIndex = index
            var value = Utils.getItemValue(comboDropdown.model, index, comboDropdown.valueRole, undefined)
            styleItem.value = value
            comboDropdown.handleItem(value)
        }
    }
