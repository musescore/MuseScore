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

Item {
    id: root

    required property int orientation
    readonly property bool isVertical: orientation === Qt.Vertical

    //! Width/height of the handle as a fraction of the total width/height
    required property real size

    readonly property bool isScrollbarNeeded: size > 0.0 && size < 1.0

    //! Position of the left/top of the handle as a fraction of the total width/height
    //! May be negative or more than 1, indicating overscroll
    required property real position

    readonly property real clampedPosition: Math.max(0, Math.min(position, 1 - size))

    //! Minimum handle size as a fraction of the total width/height
    property real minimumSize: minimumSizeInPixels / (isVertical ? height : width)
    property real minimumSizeInPixels: 30

    property real thickness: 10
    property real padding: 4

    signal moved(real newPosition)

    visible: isScrollbarNeeded

    Rectangle {
        id: handle

        property bool active: false

        color: "black"
        border.color: "white"
        border.width: 1
        radius: root.thickness / 2
        opacity: 0.0
        visible: false

        Connections {
            target: root

            function onPositionChanged() {
                handle.active = true
                Qt.callLater(function() {
                    handle.active = Qt.binding( function() { return mouseArea.containsMouse || mouseArea.pressed } )
                })
            }
        }

        states: [
            State {
                name: "active"
                when: handle.active && root.isScrollbarNeeded

                PropertyChanges {
                    target: handle
                    visible: true
                    opacity: mouseArea.pressed ? 0.7 : 0.3
                }
            }
        ]

        transitions: Transition {
            from: "active"

            SequentialAnimation {
                PauseAnimation { duration: 200 }

                NumberAnimation {
                    target: handle
                    property: "opacity"
                    duration: 100
                    to: 0.0
                }

                PropertyAction {
                    target: handle
                    property: "visible"
                }
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: root
        enabled: root.isScrollbarNeeded
        hoverEnabled: true

        property real moveStartOffset: 0.0

        //! Converts a visual position to the logical position, compensating for minimumSize
        function toLogical(clickPos) {
            let correctedClickPos = clickPos - root.padding

            if (root.minimumSize > root.size) {
                correctedClickPos *= (1.0 - root.size) / (1.0 - root.minimumSize)
            }

            return correctedClickPos / availableTrackLength
        }

        onPressed: function(mouse) {
            let clickPos = root.isVertical ? mouse.y : mouse.x
            let start = root.isVertical ? handle.y : handle.x
            let handleSize = root.isVertical ? handle.height : handle.width
            let end = start + handleSize

            if (clickPos < start || clickPos > end) {
                // TODO: when currently in overscroll mode and then clicking outside the scrollbar,
                // the notation position flickers.
                moveStartOffset = Math.max(root.size, minimumSize * (1.0 - root.size) / (1.0 - root.minimumSize)) / 2
            } else {
                moveStartOffset = toLogical(clickPos) - root.position
            }

            onPositionChanged(mouse)
        }

        onPositionChanged: function(mouse) {
            if (!pressed) {
                return
            }

            let clickPos = root.isVertical ? mouse.y : mouse.x
            root.moved(toLogical(clickPos) - moveStartOffset)
        }

        onReleased: function(mouse) {
            onPositionChanged(mouse)
            moveStartOffset = 0.0
        }
    }

    readonly property real availableTrackLength:
        (isVertical ? height : width) - 2 * padding

    readonly property real handlePosition: {
        let visualPosition = root.size < root.minimumSize
            ? clampedPosition / (1.0 - size) * (1.0 - minimumSize)
            : clampedPosition

        return visualPosition * availableTrackLength + root.padding
    }

    readonly property real handleSize:
        Math.max(root.size, root.minimumSize) * availableTrackLength

    states: [
        State {
            name: "horizontal"
            when: root.isScrollbarNeeded && !root.isVertical

            PropertyChanges {
                target: root
                anchors.rightMargin: root.thickness // avoid clash with vertical scrollbar
                height: root.thickness + 2 * root.padding
            }

            AnchorChanges {
                target: root
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
            }

            PropertyChanges {
                target: handle
                x: root.handlePosition
                y: root.padding
                width: root.handleSize
                height: root.thickness
            }
        },

        State {
            name: "vertical"
            when: root.isScrollbarNeeded && root.isVertical

            PropertyChanges {
                target: root
                anchors.bottomMargin: root.thickness // avoid clash with horizontal scrollbar
                width: root.thickness + 2 * root.padding
            }

            AnchorChanges {
                target: root
                anchors.bottom: parent.bottom
                anchors.top: parent.top
                anchors.right: parent.right
            }

            PropertyChanges {
                target: handle
                x: root.padding
                y: root.handlePosition
                width: root.thickness
                height: root.handleSize
            }
        }
    ]
}
