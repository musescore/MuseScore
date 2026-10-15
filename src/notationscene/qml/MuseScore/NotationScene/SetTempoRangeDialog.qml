/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

//! TODO: UI only for now; the values aren't applied to the tempo automation yet
StyledDialogView {
    id: root

    title: ""

    contentWidth: 368
    contentHeight: content.implicitHeight
    margins: 16

    modal: true

    property int minimumBpm: 40
    property int maximumBpm: 200

    ColumnLayout {
        id: content
        anchors.fill: parent
        spacing: 12

        NavigationPanel {
            id: rangeNavigationPanel
            name: "TempoRangeNavigationPanel"
            enabled: content.enabled && content.visible
            section: root.navigationSection
            order: 1
            direction: NavigationPanel.Horizontal
        }

        StyledTextLabel {
            Layout.fillWidth: true
            text: "Set displayed tempo range"
            font: ui.theme.bodyBoldFont
            horizontalAlignment: Text.AlignLeft
        }

        RowLayout {
            spacing: 12

            ColumnLayout {
                spacing: 8

                StyledTextLabel {
                    id: minimumLabel
                    text: "Minimum"
                    horizontalAlignment: Text.AlignLeft
                }

                IncrementalPropertyControl {
                    Layout.preferredWidth: 100

                    navigation.name: "MinimumBpm"
                    navigation.panel: rangeNavigationPanel
                    navigation.order: 0
                    navigation.accessible.name: minimumLabel.text

                    currentValue: root.minimumBpm
                    measureUnitsSymbol: "BPM"
                    step: 1
                    decimals: 0
                    minValue: 1
                    maxValue: root.maximumBpm - 1

                    onValueEdited: function(newValue) {
                        root.minimumBpm = newValue
                    }
                }
            }

            ColumnLayout {
                spacing: 8

                StyledTextLabel {
                    id: maximumLabel
                    text: "Maximum"
                    horizontalAlignment: Text.AlignLeft
                }

                IncrementalPropertyControl {
                    Layout.preferredWidth: 100

                    navigation.name: "MaximumBpm"
                    navigation.panel: rangeNavigationPanel
                    navigation.order: 1
                    navigation.accessible.name: maximumLabel.text

                    currentValue: root.maximumBpm
                    measureUnitsSymbol: "BPM"
                    step: 1
                    decimals: 0
                    minValue: root.minimumBpm + 1
                    maxValue: 999

                    onValueEdited: function(newValue) {
                        root.maximumBpm = newValue
                    }
                }
            }
        }

        ButtonBox {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignRight | Qt.AlignBottom

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 2

            FlatButton {
                text: "Apply"
                buttonRole: ButtonBoxModel.AcceptRole
                buttonId: ButtonBoxModel.Apply
                accentButton: true

                onClicked: {
                    root.hide()
                }
            }
        }
    }
}
