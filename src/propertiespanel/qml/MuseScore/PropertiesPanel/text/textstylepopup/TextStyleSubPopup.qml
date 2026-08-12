/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick

import Muse.Ui
import Muse.UiComponents

import MuseScore.PropertiesPanel

import "../../common"

StyledPopupView {
    id: root

    required property TextSettingsModel textSettingsModel

    property int navigationOrderStart: 0
    readonly property int navigationOrderEnd: textStyleSubSettingsNavPanel.order

    Column {
        width: parent.width
        spacing: 12

        NavigationPanel {
            id: textStyleSubSettingsNavPanel
            name: "TextStyleSubSettings"
            direction: NavigationPanel.Vertical
            section: root.navigationSection
            order: root.navigationOrderStart
            accessible.name: qsTrc("propertiespanel", "Additional text style controls")
        }

        DropdownPropertyView {
            id: textStyleSection
            titleText: qsTrc("propertiespanel", "Text style")
            propertyItem: root.textSettingsModel.textType

            navigationPanel: textStyleSubSettingsNavPanel
            navigationRowStart: 1

            visible: !root.textSettingsModel.isDynamicSpecificSettings
            height: implicitHeight

            model: root.textSettingsModel.textStyles
        }

        PlacementSection {
            id: textPlacementSection
            propertyItem: root.textSettingsModel.textPlacement

            navigationPanel: textStyleSubSettingsNavPanel
            navigationRowStart: textStyleSection.navigationRowEnd + 1

            visible: !root.textSettingsModel.isDynamicSpecificSettings
        }
    }
}
