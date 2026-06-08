/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

import Muse.Ui
import Muse.UiComponents
import MuseScore.PropertiesPanel

import "../common"

PropertiesPanelSection {
    id: root

    required property PartsSettingsModel model

    implicitHeight: contentColumn.height

    Column {
        id: contentColumn

        width: parent.width
        spacing: 12

        Column {
            id: linkOptions

            width: parent.width
            spacing: 12

            visible: root.model ? root.model.showPartLinkingOption : false
            height: visible ? implicitHeight : 0

            enabled: !excludeFromScore.checked

            StyledTextLabel {
                width: parent.width

                text: qsTrc("propertiespanel", "Synchronize with main score")
                horizontalAlignment: Text.AlignLeft
            }

            PropertyToggle {
                id: positionLinkedToMasterToggle
                width: parent.width

                navigation.name: "Position linked to score"
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationRowStart

                propertyItem: root.model ? root.model.positionLinkedToMaster : null
                text: qsTrc("propertiespanel", "Position")
            }

            PropertyToggle {
                id: appearanceLinkedToMasterToggle
                width: parent.width

                navigation.name: "Style / appearance linked to score"
                navigation.panel: root.navigationPanel
                navigation.row: positionLinkedToMasterToggle.navigation.row + 1

                propertyItem: root.model ? root.model.appearanceLinkedToMaster : null
                text: qsTrc("propertiespanel", "Style/appearance")
            }

            PropertyToggle {
                id: textLinkedToMasterToggle
                width: parent.width

                visible: root.model ? root.model.showTextLinkingOption : false

                navigation.name: "Text linked to score"
                navigation.panel: root.navigationPanel
                navigation.row: appearanceLinkedToMasterToggle.navigation.row + 1

                propertyItem: root.model ? root.model.textLinkedToMaster : null
                text: qsTrc("propertiespanel", "Text")
            }
        }

        PropertyCheckBox {
            id: excludeFromScore

            width: parent.width

            propertyItem: root.model ? root.model.excludeFromOtherParts : null

            visible: root.model ? root.model.showExcludeOption : false

            navigation.name: "Exclude from other parts"
            navigation.panel: root.navigationPanel
            navigation.row: textLinkedToMasterToggle.navigation.row + 1

            text: root.model && root.model.isMasterScore
                  ? qsTrc("propertiespanel", "Exclude from parts")
                  : qsTrc("propertiespanel", "Exclude from score")
        }
    }
}
