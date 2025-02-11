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
import QtQuick 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.InstrumentsScene 1.0

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
            text: qsTrc("layout", "Display at this position on the score")
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
                width: parent.width

                navigation.panel: root.navigationPanel
                navigation.name: "SystemObjectsVisibility"
                navigation.row: model.index

                text: model.title
                isVisible: model.visible

                onVisibleToggled: {
                    settingsModel.setSystemObjectsGroupVisible(model.index, !model.visible)
                }
            }
        }
    }
}
