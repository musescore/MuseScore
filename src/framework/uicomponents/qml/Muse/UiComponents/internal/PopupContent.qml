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

FocusScope {
    id: root

    default property alias contentData: contentBody.data

    property int contentWidth: 0
    property int contentHeight: 0
    property int contentBodyHeight: contentBody.childrenRect.height

    property alias background: contentBackground

    property int padding: 0
    property int margins: 12

    property bool showArrow: false
    property int arrowX: 0
    property int arrowY: 0

    property int popupPosition: PopupPosition.Bottom

    property bool isOpened: false
    property bool useDropShadow: true

    property bool animationEnabled: false

    property bool closeOnEscape: true

    width: contentContainer.width + padding * 2
    height: contentContainer.height + padding * 2

    implicitWidth: contentContainer.implicitWidth + padding * 2
    implicitHeight: contentContainer.implicitHeight + padding * 2

    focus: true

    signal closeRequested()

    //! NOTE: must to be inside QQuickItem to define a window by parent
    property NavigationSection navigationSection: NavigationSection {
        name: root.objectName !== "" ? root.objectName : "StyledPopupView" // todo
        type: NavigationSection.Exclusive
        enabled: root.isOpened
        order: 1

        onActiveChanged: {
            if (navigationSection.active) {
                root.forceActiveFocus()
            }
        }

        onNavigationEvent: function(event) {
            if (event.type === NavigationEvent.Escape && root.closeOnEscape) {
                root.closeRequested()
            }
        }
    }

    Item {
        id: contentContainer
        x: root.padding
        y: root.padding
        width: contentBody.width + root.margins * 2
        height: contentBody.height + root.margins * 2

        implicitWidth: contentBody.implicitWidth + root.margins * 2
        implicitHeight: contentBody.implicitHeight + root.margins * 2

        scale: root.animationEnabled ? 0.7 : 1.0
        opacity: root.animationEnabled ? 0.5 : 1.0
        transformOrigin: Item.Center

        ItemWithDropShadow {
            anchors.fill: parent
            shadow.radius: root.padding

            shadow.visible: root.useDropShadow

            Rectangle {
                id: contentBackground
                anchors.fill: parent

                color: ui.theme.popupBackgroundColor
                radius: 4
            }
        }

        Item {
            id: contentBody
            x: root.margins
            y: root.margins
            width: root.contentWidth
            height: root.contentHeight

            implicitWidth: root.contentWidth
            implicitHeight: root.contentHeight
        }

        Rectangle {
            id: contentBorder
            anchors.fill: parent

            color: "transparent"
            radius: contentBackground.radius
            border.width: 1
            border.color: ui.theme.strokeColor
        }

        Canvas {
            id: arrow

            height: root.popupPosition & PopupPosition.Vertical
                ? root.padding
                : 2 * root.padding;
            width: root.popupPosition & PopupPosition.Vertical
                ? 2 * root.padding
                : root.padding;

            visible: root.showArrow && arrow.height > 0
            enabled: root.showArrow

            x: {
                if (root.popupPosition & PopupPosition.Vertical) {
                    return root.arrowX - arrow.width / 2 - root.padding;
                } else if (root.popupPosition & PopupPosition.Left) {
                    return parent.x + parent.width - width - contentBackground.border.width
                } else if (root.popupPosition & PopupPosition.Right)
                    return -width + contentBackground.border.width
            }

            y: {
                if (root.popupPosition & PopupPosition.Top) {
                    return parent.y + parent.height - height - contentBackground.border.width
                } else if (root.popupPosition & PopupPosition.Bottom) {
                    return -height + contentBackground.border.width
                } else if (root.popupPosition & PopupPosition.Horizontal)
                    return root.arrowY - arrow.height / 2 - root.padding;
            }

            onPaint: {
                var ctx = getContext("2d");
                ctx.clearRect(0, 0, width, height)

                ctx.lineWidth = 2;
                ctx.fillStyle = contentBackground.color
                ctx.strokeStyle = contentBorder.border.color
                ctx.beginPath();

                if (root.popupPosition & PopupPosition.Top) {
                    ctx.moveTo(0, 0);
                    ctx.lineTo(width / 2, height - 1);
                    ctx.lineTo(width, 0);
                } else if (root.popupPosition & PopupPosition.Bottom) {
                    ctx.moveTo(0, height);
                    ctx.lineTo(width / 2, 1);
                    ctx.lineTo(width, height);
                } else if (root.popupPosition & PopupPosition.Left) {
                    ctx.moveTo(0, 0);
                    ctx.lineTo(width, height / 2);
                    ctx.lineTo(0, height);
                } else if (root.popupPosition & PopupPosition.Right) {
                    ctx.moveTo(width, 0);
                    ctx.lineTo(0, height / 2);
                    ctx.lineTo(width, height);
                }

                ctx.stroke();
                ctx.fill();
            }

            Connections {
                target: root
                function onPopupPositionChanged() { arrow.requestPaint() }
            }

            Connections {
                target: contentBackground
                function onColorChanged() { arrow.requestPaint() }
            }

            Connections {
                target: contentBorder.border
                function onColorChanged() { arrow.requestPaint() }
            }
        }
    }

    states: [
        State {
            name: "OPENED"
            when: root.isOpened
            PropertyChanges { target: contentContainer; scale: 1.0; opacity: 1.0 }
        },

        State {
            name: "CLOSED"
            when: !root.isOpened
            PropertyChanges { target: contentContainer; scale: root.animationEnabled ? 0.7 : 1.0; opacity: root.animationEnabled ? 0.5 : 1.0 }
        }
    ]

    transitions: Transition {
        NumberAnimation { properties: "scale, opacity"; easing.type: Easing.OutQuint; duration: root.animationEnabled ? 300 : 0 }
    }
}
