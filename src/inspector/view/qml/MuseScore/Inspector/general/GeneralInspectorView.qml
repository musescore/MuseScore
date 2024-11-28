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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../common"
import "playback"
import "appearance"

InspectorSectionView {
    id: root

    implicitHeight: contentColumn.height

    enabled: model ? model.areGeneralPropertiesAvailable : true

    ColumnLayout {
        id: contentColumn

        width: parent.width

        spacing: 12

        GridLayout {
            width: parent.width

            columns: 2

            rowSpacing: 12
            columnSpacing: 4

            PropertyCheckBox {
                Layout.fillWidth: true
                Layout.maximumWidth: parent.width / 2

                navigation.name: "Visible"
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationRow(1)

                text: qsTrc("inspector", "Visible")
                propertyItem: root.model ? root.model.isVisible : null
                isIndeterminate: enabled && propertyItem && propertyItem.isUndefined
            }

            PropertyCheckBox {
                Layout.fillWidth: true
                Layout.maximumWidth: parent.width / 2

                navigation.name: "Cue size"
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationRow(2)

                text: qsTrc("inspector", "Cue size")
                propertyItem: root.model ? root.model.isSmall : null
                isIndeterminate: enabled && propertyItem && propertyItem.isUndefined
            }

            PropertyCheckBox {
                Layout.fillWidth: true
                Layout.maximumWidth: parent.width / 2

                navigation.name: "Auto-place"
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationRow(3)

                text: qsTrc("inspector", "Auto-place")
                propertyItem: root.model ? root.model.isAutoPlaceAllowed : null
                isIndeterminate: enabled && propertyItem && propertyItem.isUndefined
            }

            PropertyCheckBox {
                Layout.fillWidth: true
                Layout.maximumWidth: parent.width / 2

                navigation.name: "Play"
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationRow(4)

                text: qsTrc("inspector", "Play")
                propertyItem: root.model ? root.model.isPlayable : null
                isIndeterminate: enabled && propertyItem && propertyItem.isUndefined
            }
        }

        GridLayout {
            width: parent.width

            columns: 2
            columnSpacing: 4

            PopupViewButton {
                id: playbackButton

                popupAvailableWidth: parent ? parent.width : 0
                anchorItem: root.anchorItem

                navigation.panel: root.navigationPanel
                navigation.name: "Playback"
                navigation.row: root.navigationRow(5)

                icon: IconCode.AUDIO
                text: qsTrc("inspector", "Playback")

                enabled: model && !model.playbackProxyModel.isEmpty

                popupContent: PlaybackSettings {
                    proxyModel: model ? model.playbackProxyModel : null

                    navigationPanel: playbackButton.popupNavigationPanel
                }

                onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                    root.ensureContentVisibleRequested(invisibleContentHeight)
                }

                onPopupOpened: function(openedPopup, control) {
                    root.popupOpened(popup, control)
                }
            }

            PopupViewButton {
                id: appearanceButton

                popupAvailableWidth: parent ? parent.width : 0
                anchorItem: root.anchorItem

                navigation.panel: root.navigationPanel
                navigation.name: "Appearance"
                navigation.row: root.navigationRow(6)

                icon: IconCode.POSITION_ARROWS
                text: qsTrc("inspector", "Appearance")

                enabled: model && !model.appearanceSettingsModel.isEmpty

                popupContent: AppearanceSettings {
                    model: root.model ? root.model.appearanceSettingsModel : null

                    navigationPanel: appearanceButton.popupNavigationPanel
                }

                onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                    root.ensureContentVisibleRequested(invisibleContentHeight)
                }

                onPopupOpened: function(popup, control) {
                    root.popupOpened(popup, control)
                }
            }
        }
    }
}
