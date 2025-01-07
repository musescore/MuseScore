/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.NotationScene 1.0

StyledPopupView {
    id: root

    property alias notationViewNavigationSection: navPanel.section
    property alias navigationOrderStart: navPanel.order
    readonly property alias navigationOrderEnd: navPanel.order

    property QtObject model: stringTuningsModel


    contentWidth: content.width
    contentHeight: content.height

    showArrow: false

    signal elementRectChanged(var elementRect)

    function updatePosition() {
        var h = Math.max(root.contentHeight, 360)
        root.x = root.parent.width + 12
        root.y = (root.parent.y + (root.parent.height / 2)) - root.parent.y - h / 2 + root.padding * 2
    }

    ColumnLayout {
        id: content

        width: 272

        spacing: 12

        StringTuningsSettingsModel {
            id: stringTuningsModel

            onItemRectChanged: function(rect) {
                root.elementRectChanged(rect)
            }
        }

        Component.onCompleted: {
            stringTuningsModel.init()
        }

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
                navigation.accessible.name: titleLabel.text + " " + currentText

                model: stringTuningsModel.presets

                currentIndex: indexOfText(stringTuningsModel.currentPreset)

                onActivated: function(index, value) {
                    stringTuningsModel.currentPreset = textOfValue(value)
                }
            }

            StyledDropdown {
                id: stringNumberDropdown

                Layout.preferredWidth: 92

                navigation.name: "StringsNumber"
                navigation.panel: navPanel
                navigation.row: 2
                navigation.accessible.name: qsTrc("notation", "Number of strings:") + " " + currentText

                model: stringTuningsModel.numbersOfStrings

                currentIndex: indexOfValue(stringTuningsModel.currentNumberOfStrings)

                onActivated: function(index, value) {
                    stringTuningsModel.currentNumberOfStrings = parseInt(textOfValue(value))
                }
            }
        }

        NavigationPanel {
            id: stringsNavPanel
            name: "StringTuningsSettings"
            direction: NavigationPanel.Vertical
            section: root.notationViewNavigationSection
            order: navPanel.order + 1
            accessible.name: qsTrc("notation", "Strings")
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
                    navigation.accessible.name: visibleBox.navigation.accessible.name + " " + qsTrc("notation", "Value %1").arg(valueControl.currentValue)

                    RowLayout {
                        anchors.fill: parent

                        VisibilityBox {
                            id: visibleBox

                            isVisible: modelData["show"]

                            navigation.panel: stringsNavPanel
                            navigation.row: index
                            navigation.column: 2
                            accessibleText: qsTrc("notation", "String %1").arg(numberLabel.text)

                            onVisibleToggled: {
                                stringTuningsModel.toggleString(index)
                            }
                        }

                        Rectangle {
                            height: numberLabel.height + 4
                            width: height

                            color: "transparent"
                            radius: 180
                            border.color: ui.theme.fontPrimaryColor
                            border.width: 1

                            StyledTextLabel {
                                id: numberLabel

                                anchors.centerIn: parent

                                text: modelData["number"]
                            }
                        }

                        IncrementalPropertyControl {
                            id: valueControl

                            Layout.leftMargin: 6

                            Layout.preferredHeight: parent.height - ui.theme.borderWidth * 2
                            Layout.preferredWidth: 64

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
}
