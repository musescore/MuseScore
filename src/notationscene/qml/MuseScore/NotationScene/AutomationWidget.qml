/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

import Muse.Ui
import Muse.UiComponents
import MuseScore.NotationScene

// On-score automation type switcher (the "Widget" automation UI variant)
Item {
    id: root

    //! NOTE: in the parent's coordinates: { x, y } is the top-left of the system's first visible staff,
    //! { bottom } is the bottom of the system
    property var anchorData: null

    // Distance from the staff, and from the view's edges when the system's top-left is scrolled out of view
    readonly property int margin: 12

    visible: widgetModel.isVisible && Boolean(root.anchorData)

    // Above the automation lines
    z: 1

    width: button.width
    height: button.height

    // Sticks to the view's top-left edges when the system's top-left is scrolled out of view,
    // but never leaves its system: it slides off along with the system's bottom
    x: Math.max(root.margin, prv.anchorX)
    y: {
        const desiredY = prv.anchorY - root.height - root.margin
        const stickyY = Math.max(root.margin, desiredY)
        return Math.min(stickyY, Math.max(desiredY, prv.systemBottom - root.height))
    }

    QtObject {
        id: prv

        // Assigned imperatively (not bound), so the widget can stay put while the menu is open
        property real anchorX: 0
        property real anchorY: 0
        property real systemBottom: 0

        function updateAnchor() {
            if (menuLoader.isMenuOpened || !root.anchorData) {
                return
            }

            prv.anchorX = root.anchorData.x
            prv.anchorY = root.anchorData.y
            prv.systemBottom = root.anchorData.bottom
        }
    }

    onAnchorDataChanged: prv.updateAnchor()

    AutomationWidgetModel {
        id: widgetModel
    }

    Component.onCompleted: {
        widgetModel.load()
        prv.updateAnchor()
    }

    // Same shadow as in-score popups (e.g. the sound flag popup)
    StyledRectangularShadow {
        anchors.fill: button

        blur: 12
        radius: button.backgroundRadius
    }

    // Opaque base, so the (semi-transparent) button background doesn't show the score through it
    Rectangle {
        anchors.fill: button

        color: ui.theme.backgroundPrimaryColor
        radius: button.backgroundRadius
    }

    FlatButton {
        id: button

        height: 30

        orientation: Qt.Horizontal
        icon: IconCode.AUTOMATION
        text: widgetModel.currentTypeTitle

        accentButton: menuLoader.isMenuOpened

        onClicked: {
            if (!menuLoader.isMenuOpened) {
                widgetModel.load()
            }

            menuLoader.toggleOpened(widgetModel.items, button.width, 0)
        }

        StyledMenuLoader {
            id: menuLoader

            onHandleMenuItem: function(itemId) {
                widgetModel.handleMenuItem(itemId)
            }

            onClosed: prv.updateAnchor()
        }
    }
}
