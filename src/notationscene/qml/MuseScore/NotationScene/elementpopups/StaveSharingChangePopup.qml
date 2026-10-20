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
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.NotationScene

AbstractElementPopup {
    id: root

    property alias notationViewNavigationSection: staveSharingChangeNavPanel.section
    property alias navigationOrderStart: staveSharingChangeNavPanel.order
    readonly property alias navigationOrderEnd: staveSharingChangeNavPanel.order

    contentWidth: content.width
    contentHeight: content.height

    placementPolicies: PopupView.PreferRight
    showArrow: false

    model: StaveSharingChangePopupModel {
        id: popupModel
    }

    function updatePosition() {
        root.y = 0
        Qt.callLater(root.repositionWindowIfNeed)
    }

    ColumnLayout {
        id: content

        width: 200

        spacing: 4

        NavigationPanel {
            id: staveSharingChangeNavPanel
            name: "StaveSharingChange"
            direction: NavigationPanel.Vertical
            accessible.name: qsTrc("notation", "Stave sharing change")

            onNavigationEvent: function(event) {
                if (event.type === NavigationEvent.Escape) {
                    root.close()
                }
            }
        }

        StyledTextLabel {
            id: titleLabel

            text: qsTrc("notation", "Stave sharing change")
            font: ui.theme.largeBodyBoldFont
            horizontalAlignment: Text.AlignLeft
        }

        StyledTextLabel {
            id: sharedPartNameLabel

            text: popupModel.sharedPartName
            font: ui.theme.bodyFont
            horizontalAlignment: Text.AlignLeft
        }

        StyledTextLabel {
            id: userMapValidWarning

            text: qsTrc("notation", "This stave configuration is invalid")
            font: ui.theme.bodyFont
            horizontalAlignment: Text.AlignLeft

            visible: !popupModel.isUserMapValid
        }

        SeparatorLine {}

        ToggleButton {
            id: resetToDefaultButton

            text: qsTrc("notation", "Reset to default")

            navigation.name: "ResetToDefault"
            navigation.panel: staveSharingChangeNavPanel
            navigation.row: 1

            checked: popupModel.resetToDefault

            onToggled: {
                popupModel.resetToDefault = !checked
            }
        }

        ColumnLayout {
            Layout.fillWidth: true

            visible: !popupModel.resetToDefault

            spacing: 4

            Repeater {
                model: popupModel.trackMappings

                RowLayout {
                    id: mappingRow

                    required property var modelData
                    required property int index

                    Layout.fillWidth: true

                    spacing: 6

                    StyledTextLabel {
                        Layout.fillWidth: true

                        text: mappingRow.modelData.originPartName
                        horizontalAlignment: Text.AlignLeft
                    }

                    StyledDropdown {
                        Layout.preferredWidth: 56

                        navigation.name: "Stave" + mappingRow.index
                        navigation.panel: staveSharingChangeNavPanel
                        navigation.row: 2 + mappingRow.index * 2

                        model: popupModel.staveOptions

                        currentIndex: indexOfValue(mappingRow.modelData.staveIndex)

                        onActivated: function(index, value) {
                            popupModel.setTrackMappingStave(mappingRow.index, parseInt(value))
                        }
                    }

                    StyledDropdown {
                        Layout.preferredWidth: 56

                        navigation.name: "Voice" + mappingRow.index
                        navigation.panel: staveSharingChangeNavPanel
                        navigation.row: 2 + mappingRow.index * 2 + 1

                        model: popupModel.voiceOptions

                        currentIndex: indexOfValue(mappingRow.modelData.voiceIndex)

                        onActivated: function(index, value) {
                            popupModel.setTrackMappingVoice(mappingRow.index, parseInt(value))
                        }
                    }
                }
            }
        }

    }
}
