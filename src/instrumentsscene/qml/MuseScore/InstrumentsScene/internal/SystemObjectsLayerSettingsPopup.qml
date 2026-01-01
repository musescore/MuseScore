/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

import Muse.Ui
import Muse.UiComponents
import MuseScore.InstrumentsScene

StyledPopupView {
    id: root

    property bool needActiveFirstItem: false

    contentHeight: contentColumn.childrenRect.height

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "SystemObjectsLayerSettingsPopup"
        section: root.navigationSection
        order: 1
        direction: NavigationPanel.Vertical
    }

    function load(obj) {
        settingsModel.load(obj.staffId)
    }

    SystemObjectsLayerSettingsModel {
        id: settingsModel
    }

    Column {
        id: contentColumn

        anchors.fill: parent
        spacing: 12

        StyledTextLabel {
            text: qsTrc("layoutpanel", "Display at this position on the score")
            font: ui.theme.bodyBoldFont
        }

        StyledListView {
            id: systemObjectsView

            width: parent.width
            height: contentHeight

            spacing: 4
            interactive: false

            model: settingsModel

            delegate: VisibilityBox {
                required property string title
                required property bool isGroupVisible
                required property int index

                width: parent.width

                navigation.panel: root.navigationPanel
                navigation.name: "SystemObjectsVisibility"
                navigation.row: index

                text: title
                isVisible: isGroupVisible

                onVisibleToggled: {
                    settingsModel.setSystemObjectsGroupVisible(index, !isGroupVisible)
                }
            }
        }
    }
}
