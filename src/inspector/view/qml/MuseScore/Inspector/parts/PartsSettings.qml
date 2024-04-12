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

InspectorSectionView {
    id: root

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

                text: qsTrc("inspector", "Synchronize with main score")
                horizontalAlignment: Text.AlignLeft
            }

            PropertyToggle {
                id: positionLinkedToMasterToggle

                navigation.name: "Position linked to score"
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationRowStart

                propertyItem: root.model ? root.model.positionLinkedToMaster : null
                text: qsTrc("inspector", "Position")
            }

            PropertyToggle {
                id: appearanceLinkedToMasterToggle

                navigation.name: "Style / appearance linked to score"
                navigation.panel: root.navigationPanel
                navigation.row: positionLinkedToMasterToggle.navigationRow + 1

                propertyItem: root.model ? root.model.appearanceLinkedToMaster : null
                text: qsTrc("inspector", "Style/appearance")
            }

            PropertyToggle {
                id: textLinkedToMasterToggle

                visible: root.model ? root.model.showTextLinkingOption : false

                navigation.name: "Text linked to score"
                navigation.panel: root.navigationPanel
                navigation.row: appearanceLinkedToMasterToggle.navigationRow + 1

                propertyItem: root.model ? root.model.textLinkedToMaster : null
                text: qsTrc("inspector", "Text")
            }
        }

        PropertyCheckBox {
            id: excludeFromScore

            width: parent.width

            propertyItem: root.model ? root.model.excludeFromOtherParts : null

            visible: root.model ? root.model.showExcludeOption : false

            navigation.name: "Exclude from other parts"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart

            text: root.model && root.model.showPartLinkingOption
                  ? qsTrc("inspector", "Exclude from score")
                  : qsTrc("inspector", "Exclude from parts")
        }
    }
}
