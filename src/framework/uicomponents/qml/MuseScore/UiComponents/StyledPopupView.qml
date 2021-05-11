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

PopupView {
    id: root

    default property alias contentData: contentBody.data

    property alias background: contentBackground

    property alias width: rootContainer.width
    property alias height: rootContainer.height

    property int padding: 12
    property int margins: 16

    property int contentWidth: 240
    property int contentHeight: contentBody.childrenRect.height

    property bool opensUpward: false
    property int arrowX: root.width / 2
    property bool showArrow: true

    property bool animationEnabled: false

    property alias navigation: keynavPanel
    property bool isDoActiveParentOnClose: true

    closePolicy: PopupView.CloseOnPressOutsideParent

    x: (root.parent.width / 2) - (root.width / 2)
    y: root.opensUpward ? -root.height : root.parent.height

    property NavigationPanel keynavPanel: NavigationPanel {
        id: keynavPanel
        enabled: root.isOpened
        order: {
            var pctrl = root.navigationParentControl;
            if (pctrl) {
                if (pctrl.panel) {
                    return pctrl.panel.order + 1
                }
            }
            return -1
        }

        section: {
            var pctrl = root.navigationParentControl;
            if (pctrl) {
                if (pctrl.panel) {
                    return pctrl.panel.section
                }
            }
            return null
        }

        onActiveChanged: {
            if (keynavPanel.active) {
                root.forceActiveFocus()
                rootContainer.forceActiveFocus()
            } else {
                root.close()
            }
        }

        onNavigationEvent: {
            if (event.type === NavigationEvent.Escape) {
                root.close()
            }
        }
    }

    onClosed: {
        rootContainer.focus = false
        if (root.isDoActiveParentOnClose && root.navigationParentControl) {
            root.navigationParentControl.forceActive()
        }
    }

    contentItem: FocusScope {
        id: rootContainer
        width: contentContainer.width + root.padding * 2
        height: contentContainer.height + root.padding * 2

        implicitWidth: contentContainer.implicitWidth + root.padding * 2
        implicitHeight: contentContainer.implicitHeight + root.padding * 2

        focus: true

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

            Rectangle {
                id: contentBackground
                anchors.fill: parent
                color: ui.theme.backgroundPrimaryColor
                border.width: 1
                border.color: ui.theme.strokeColor
            }

            StyledDropShadow {
                anchors.fill: parent
                source: contentBackground
            }

            Canvas {
                id: arrow
                anchors.top: root.opensUpward ? undefined : parent.top
                anchors.topMargin: root.opensUpward ? 0 : (-arrow.height + contentBackground.border.width)
                anchors.bottom: root.opensUpward ? parent.bottom : undefined
                anchors.bottomMargin: root.opensUpward ? (-arrow.height + contentBackground.border.width) : 0
                height: root.padding
                width: root.padding * 2
                visible: root.showArrow && arrow.height > 0
                enabled: root.showArrow
                x: root.arrowX - arrow.width / 2 - root.padding

                onPaint: {
                    var ctx = getContext("2d");
                    ctx.lineWidth = 2;
                    ctx.fillStyle = contentBackground.color
                    ctx.strokeStyle = contentBackground.border.color
                    ctx.beginPath();

                    if (opensUpward) {
                        ctx.moveTo(0, 0);
                        ctx.lineTo(width / 2, height - 1);
                        ctx.lineTo(width, 0);
                    } else {
                        ctx.moveTo(0, height);
                        ctx.lineTo(width / 2, 1);
                        ctx.lineTo(width, height);
                    }

                    ctx.stroke();
                    ctx.fill();
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
}
