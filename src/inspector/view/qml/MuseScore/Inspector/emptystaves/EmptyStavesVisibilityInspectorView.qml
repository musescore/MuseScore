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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../common"

InspectorSectionView {
    id: root

    implicitHeight: contentColumn.implicitHeight

    headerAccessory: PropertyResetButton {
        navigation.name: "ResetEmptyStavesVisibility"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRowStart + 3

        toolTipTitle: qsTrc("inspector", "Reset to default")
        enabled: root.model && root.model.canResetEmptyStavesVisibility

        onClicked: {
            if (root.model) {
                root.model.resetEmptyStavesVisibility()
            }
        }
    }

    Column {
        id: contentColumn

        width: parent.width
        spacing: 12

        FlatButton {
            width: parent.width

            navigation.name: "HideEmptyStaves"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart + 1

            icon: IconCode.HIDE_EMPTY_STAVES
            text: qsTrc("inspector", "Hide empty staves")
            orientation: Qt.Horizontal
            enabled: root.model && root.model.canHideEmptyStavesInSelection

            onClicked: {
                if (root.model) {
                    root.model.hideEmptyStavesInSelection()
                }
            }
        }

        FlatButton {
            width: parent.width

            navigation.name: "ShowAllEmptyStaves"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart + 2

            icon: IconCode.SHOW_EMPTY_STAVES
            text: qsTrc("inspector", "Show empty staves")
            orientation: Qt.Horizontal
            enabled: root.model && root.model.canShowAllEmptyStaves

            onClicked: {
                if (root.model) {
                    root.model.showAllEmptyStaves()
                }
            }
        }
    }
}
