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
import QtQuick.Layouts 1.3

import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../common"

InspectorSectionView {
    id: root

    implicitHeight: grid.implicitHeight

    GridLayout {
        id: grid

        width: parent.width

        columns: 2
        columnSpacing: 4

        Repeater {
            model: root.model ? root.model.models() : []

            delegate: PopupViewButton {
                id: button

                popupAvailableWidth: parent ? parent.width : 0
                anchorItem: root.anchorItem

                icon: modelData["icon"]
                text: modelData["title"]

                visible: !modelData["isEmpty"]

                navigation.panel: root.navigationPanel
                navigation.name: loader.viewObjectName
                navigation.row: root.navigationRow(index)

                popupContent: NotationInspectorSectionLoader {
                    id: loader
                    width: parent.width

                    model: modelData
                }

                onEnsureContentVisibleRequested: {
                    root.ensureContentVisibleRequested(invisibleContentHeight)
                }
            }
        }
    }
}
