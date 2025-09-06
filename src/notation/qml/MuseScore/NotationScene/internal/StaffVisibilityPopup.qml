/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.NotationScene

StyledPopupView {
    id: root

    property alias notationViewNavigationSection: staffVisibilityNavPanel.section
    property alias navigationOrderStart: staffVisibilityNavPanel.order
    readonly property int navigationOrderEnd: 0

    contentWidth: 276
    contentHeight: Math.min(contentColumn.implicitHeight, 600)

    margins: 0

    placementPolicies: PopupView.PreferBelow

    signal elementRectChanged(var elementRect)

    function updatePosition() {
        root.y = root.parent.height + 4; // 4 for spacing
    }

    NavigationPanel {
        id: staffVisibilityNavPanel
        name: "StaffVisibility"
        direction: NavigationPanel.Horizontal
        accessible.name: qsTrc("notation/staffvisibilitypopup", "Staff visibility popup")

        onNavigationEvent: function (event) {
            if (event.type === NavigationEvent.Escape) {
                root.close();
            }
        }
    }

    StaffVisibilityPopupModel {
        id: popupModel
        onItemRectChanged: function (rect) {
            root.elementRectChanged(rect);
        }
    }

    EmptyStavesVisibilityView {
        id: contentColumn

        anchors.fill: parent
        
        emptyStavesVisibilityModel: popupModel.emptyStavesVisibilityModel
        navigationPanel: staffVisibilityNavPanel
        systemIndex: popupModel.systemIndex
    }
}
