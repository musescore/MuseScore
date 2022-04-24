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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

import "internal"

PopupView {
    id: root

    default property alias contentData: rootContainer.contentData

    property alias background: rootContainer.background

    property alias width: rootContainer.width
    property alias height: rootContainer.height

    property alias margins: rootContainer.margins

    property alias animationEnabled: rootContainer.animationEnabled

    property alias isCloseByEscape: rootContainer.isCloseByEscape
    property alias navigationSection: rootContainer.navigationSection

    contentWidth: 240
    contentHeight: rootContainer.contentBodyHeight

    closePolicy: PopupView.CloseOnPressOutsideParent

    x: (root.parent.width / 2) - (root.width / 2)
    y: root.parent.height

    onOpened: {
        rootContainer.navigationSection.requestActive()
    }

    onClosed: {
        rootContainer.focus = false
    }

    contentItem: PopupContent {
        id: rootContainer

        contentWidth: root.contentWidth
        contentHeight: root.contentHeight

        padding: root.padding

        showArrow: root.showArrow
        arrowX: root.arrowX
        opensUpward: root.opensUpward
        isOpened: root.isOpened
    }
}
