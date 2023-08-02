/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

StyledPopupView {
    id: root

    property NavigationSection notationViewNavigationSection: null
    property int navigationOrderStart: 0
    property int navigationOrderEnd: navPanel.order

    contentWidth: content.width
    contentHeight: content.height

    showArrow: false

    function updatePosition(elementRect) {
        var h = Math.max(root.contentHeight, 360)
        root.x = elementRect.x + elementRect.width + 12
        root.y = elementRect.y - h / 2
    }

    StringTuningsSettingsModel {
        id: stringTuningsModel

        onItemRectChanged: function(rect) {
            updatePosition(rect)
        }
    }

    Component.onCompleted: {
        stringTuningsModel.init()
    }

    ColumnLayout {
        id: content

        width: 272

        spacing: 12

        NavigationPanel {
            id: navPanel
            name: "StringTuningsSettings"
            direction: NavigationPanel.Vertical
            section: root.notationViewNavigationSection
            order: root.navigationOrderStart
            accessible.name: qsTrc("notation", "String tunings settings")
        }

        StyledTextLabel {
            id: titleLabel

            text: qsTrc("notation", "Presets")
            horizontalAlignment: Text.AlignLeft
        }

        RowLayout {
            Layout.preferredWidth: parent.width

            spacing: 6

            StyledDropdown {
                id: presetsDropdown

                Layout.fillWidth: true

                navigation.name: "Presets"
                navigation.panel: navPanel
                navigation.row: 1

                model: stringTuningsModel.presets

                currentIndex: indexOfText(stringTuningsModel.currentPreset)

                onActivated: function(index, value) {
                    stringTuningsModel.currentPreset = textOfValue(value)
                }
            }

            StyledDropdown {
                id: stringNumberDropdown

                Layout.preferredWidth: 92

                navigation.name: "StringNumber"
                navigation.panel: navPanel
                navigation.row: 2

                model: stringTuningsModel.stringNumbers

                currentIndex: indexOfValue(stringTuningsModel.currentStringNumber)

                onActivated: function(index, value) {
                    stringTuningsModel.currentStringNumber = parseInt(textOfValue(value))
                }
            }
        }

        NavigationPanel {
            id: stringsNavPanel
            name: "StringTuningsSettings"
            direction: NavigationPanel.Vertical
            section: root.notationViewNavigationSection
            order: navPanel.order + 1
            accessible.name: qsTrc("notation", "Strings") // todo
        }

        GridLayout {
            id: gridView

            Layout.preferredWidth: parent.width

            readonly property int cellRadius: 2

            flow: GridLayout.TopToBottom
            columns: 2
            rows: Math.ceil(stringTuningsModel.strings.length / 2)
            columnSpacing: 10
            rowSpacing: 6

            Repeater {
                id: repeaterStrings

                width: parent.width

                model: stringTuningsModel.strings

                ListItemBlank {
                    implicitHeight: visibleBox.height
                    implicitWidth: (content.width / 2 - gridView.columnSpacing / gridView.columns)

                    hoverHitColor: "transparent"
                    background.radius: gridView.cellRadius

                    navigation.name: "String" + index
                    navigation.panel: stringsNavPanel
                    navigation.row: index
                    navigation.column: 1
                    //                navigation.accessible.name: hint // todo

                    VisibilityBox {
                        id: visibleBox

                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter

                        isVisible: modelData["show"]
                        text: modelData["number"] // todo

                        navigation.panel: stringsNavPanel
                        navigation.row: index
                        navigation.column: 2

                        onVisibleToggled: {
                            stringTuningsModel.toggleString(index)
                        }
                    }

                    IncrementalPropertyControl {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter

                        implicitHeight: parent.height - ui.theme.borderWidth * 2
                        implicitWidth: 64

                        currentValue: modelData["valueStr"]

                        minValue: 0
                        maxValue: 127

                        navigation.panel: stringsNavPanel
                        navigation.row: index
                        navigation.column: 3

                        canIncrease: modelData["value"] < maxValue
                        onIncrement: function() {
                            return stringTuningsModel.increaseStringValue(currentValue)
                        }

                        canDecrease: modelData["value"] > minValue
                        onDecrement: function() {
                            return stringTuningsModel.decreaseStringValue(currentValue)
                        }

                        onValueEditingFinished: function(newValue) {
                            var ok = stringTuningsModel.setStringValue(index, newValue)
                            if (!ok) {
                                //! NOTE: reset the text entered by the user
                                currentValue = modelData["valueStr"]
                                currentValue = Qt.binding( function() { return modelData["valueStr"] } )
                            }
                        }
                    }
                }
            }
        }
    }
}
