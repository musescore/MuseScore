/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Item {
    id: root
    enabled: selectionFilterModel.enabled

    property NavigationSection navigationSection: null

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "SelectionFilter"
        section: root.navigationSection
        direction: NavigationPanel.Vertical
        enabled: root.enabled && root.visible
        order: 2
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

            navigation.panel: root.navigationPanel
            navigation.order: model.index

            checked: model.isSelected
            isIndeterminate: model.isIndeterminate
            onClicked: {
                model.isSelected = !checked
            }
        }
    }
}
