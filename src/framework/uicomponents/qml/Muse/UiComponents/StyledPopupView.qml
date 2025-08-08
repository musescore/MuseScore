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

import Muse.Ui 1.0
import Muse.UiComponents 1.0

import "internal"

PopupView {
    id: root

    default property alias contentData: content.contentData

    property alias background: content.background

    property alias width: content.width
    property alias height: content.height

    property alias margins: content.margins

    property alias animationEnabled: content.animationEnabled

    property alias closeOnEscape: content.closeOnEscape
    property alias navigationSection: content.navigationSection

    property alias useDropShadow: content.useDropShadow

    property bool isPlacementVertical: (root.placementPolicies === PopupView.Default)
                                        || (root.placementPolicies === PopupView.PreferAbove)
                                        || (root.placementPolicies === PopupView.PreferBelow)

    contentWidth: 240
    contentHeight: content.contentBodyHeight

    closePolicies: PopupView.CloseOnPressOutsideParent

    x: root.isPlacementVertical ? (root.parent.width / 2) - (root.width / 2) : root.parent.width
    y: root.isPlacementVertical ? root.parent.height : (root.parent.height / 2) - (root.height / 2)

    onOpened: {
        if (!(openPolicies & PopupView.NoActivateFocus) && content.navigationSection) {
            content.navigationSection.requestActive()
        }
    }

    onClosed: {
        content.focus = false
    }

    contentItem: PopupContent {
        id: content

        objectName: "Popup"

        contentWidth: root.contentWidth
        contentHeight: root.contentHeight

        padding: root.padding

        showArrow: root.showArrow
        arrowX: root.arrowX
        arrowY: root.arrowY
        popupPosition: root.popupPosition
        isOpened: root.isOpened

        enabled: root.isOpened

        onCloseRequested: {
            root.close()
        }
    }
}
