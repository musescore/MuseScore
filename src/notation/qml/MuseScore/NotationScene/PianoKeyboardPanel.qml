/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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
import MuseScore.NotationScene 1.0

Item {
    id: root

    property alias contextMenuModel: contextMenuModel

    property alias navigationSection: navPanel.section
    property alias contentNavigationPanelOrderStart: navPanel.order

    NavigationPanel {
        id: navPanel
        name: "PianoKeyboardSection"
        direction: NavigationPanel.Vertical
        enabled: root.enabled && root.visible
    }

    PianoKeyboardPanelContextMenuModel {
        id: contextMenuModel

        keyWidthScaling: keyboardView.keyWidthScaling

        onSetKeyWidthScalingRequested: function(scaling) {
            keyboardView.keyWidthScaling = scaling
        }
    }

    Component.onCompleted: {
        keyboardView.init()
        contextMenuModel.load()
    }

    PinchArea {
        anchors.fill: parent

        onPinchUpdated: function(pinch) {
            keyboardView.scale(pinch.scale / pinch.previousScale, pinch.center.x)
        }

        PianoKeyboardView {
            id: keyboardView
            anchors.fill: parent

            numberOfKeys: contextMenuModel.numberOfKeys

            //! NOTE: bidirectional binding; C++ code should guard against loops
            scrollBarPosition: scrollBar.position

            StyledScrollBar {
                id: scrollBar

                orientation: Qt.Horizontal

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                color: "black"
                border.color: "white"
                border.width: 1

                position: keyboardView.scrollBarPosition
                size: keyboardView.scrollBarSize

                onPositionChanged: {
                    activate()
                }

                onSizeChanged: {
                    activate()
                }
            }
        }
    }
}
