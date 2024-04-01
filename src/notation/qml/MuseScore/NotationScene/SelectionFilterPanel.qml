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
import QtQuick.Controls 2.15

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

Item {
    id: root
    enabled: selectionFilterModel.enabled

    property alias navigationSection: navPanel.section
    property alias navigationOrderStart: navPanel.order

    NavigationPanel {
        id: navPanel
        name: "SelectionFilter"
        direction: NavigationPanel.Vertical
        enabled: root.enabled && root.visible
    }

    Component.onCompleted: {
        selectionFilterModel.load()
    }

    StyledListView {
        anchors.fill: parent

        topMargin: 12
        leftMargin: topMargin
        rightMargin: topMargin
        bottomMargin: topMargin

        spacing: 12

        model: SelectionFilterModel {
            id: selectionFilterModel
        }

        delegate: CheckBox {
            width: ListView.view.width

            text: model.title

            navigation.panel: navPanel
            navigation.order: model.index

            checked: model.isSelected
            isIndeterminate: model.isIndeterminate
            onClicked: {
                model.isSelected = !checked
            }
        }
    }
}
