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
import MuseScore.NotationScene 1.0

import "../common"

InspectorSectionView {
    id: root

    implicitHeight: contentColumn.implicitHeight

    Column {
        id: contentColumn

        width: parent.width
        spacing: 12

        PopupViewButton {
            width: parent.width

            popupAvailableWidth: parent ? parent.width : 0
            anchorItem: root.anchorItem

            navigation.name: "EmptyStavesVisibility"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart + 1

            icon: IconCode.STAFF_TYPE_CHANGE
            text: qsTrc("inspector", "Show/hide empty staves")
            orientation: Qt.Horizontal

            popup.contentHeight: Math.min(popup.contentData[0].implicitHeight, 600)
            popup.margins: 0

            popupContent: EmptyStavesVisibilityView {
                emptyStavesVisibilityModel: root.model?.emptyStavesVisibilityModel
                navigationPanel: root.navigationPanel
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
