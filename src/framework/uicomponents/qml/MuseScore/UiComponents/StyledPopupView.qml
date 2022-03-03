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

    property int margins: 12 + contentBackground.border.width

    contentWidth: 240
    contentHeight: contentBody.childrenRect.height

    property bool animationEnabled: false

    closePolicy: PopupView.CloseOnPressOutsideParent

    x: (root.parent.width / 2) - (root.width / 2)
    y: root.parent.height

    property bool isDoActiveParentOnClose: true
    property bool isCloseByEscape: true
    property NavigationSection navigationSection: NavigationSection {
        id: navSec
        name: root.objectName !== "" ? root.objectName : "StyledPopupView"
        type: NavigationSection.Exclusive
        enabled: root.isOpened
        order: 1

        onActiveChanged: {
            if (navSec.active) {
                rootContainer.forceActiveFocus()
            }
        }

        onNavigationEvent: function(event) {
            if (event.type === NavigationEvent.Escape && root.isCloseByEscape) {
                root.close()
            }
        }
    }

    onOpened: {
        navSec.requestActive()
    }

    onClosed: {
        rootContainer.focus = false
        if (root.isDoActiveParentOnClose && root.navigationParentControl) {
            root.navigationParentControl.requestActive()
        }
    }

    contentItem: FocusScope {
        id: rootContainer
        width: contentContainer.width + root.padding * 2
        height: contentContainer.height + root.padding * 2

        implicitWidth: contentContainer.implicitWidth + root.padding * 2
        implicitHeight: contentContainer.implicitHeight + root.padding * 2

        property alias cascadeAlign: root.cascadeAlign

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

            StyledDropShadow {
                anchors.fill: parent
                anchors.topMargin: root.padding / 2
                anchors.bottomMargin: root.padding / 2
                source: contentBackground
            }

            Rectangle {
                id: contentBackground
                anchors.fill: parent
                color: ui.theme.backgroundPrimaryColor
                radius: 4
                border.width: 1
                border.color: ui.theme.strokeColor
            }

            Canvas {
                id: arrow

                height: root.padding
                width: root.padding * 2

                visible: root.showArrow && arrow.height > 0
                enabled: root.showArrow

                x: root.arrowX - arrow.width / 2 - root.padding
                y: root.opensUpward ? parent.y + parent.height - height - contentBackground.border.width
                                    : -height + contentBackground.border.width

                onPaint: {
                    var ctx = getContext("2d");
                    ctx.clearRect(0, 0, width, height)

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

                Connections {
                    target: root
                    function onOpensUpwardChanged() { arrow.requestPaint() }
                }

                Connections {
                    target: contentBackground
                    function onColorChanged() { arrow.requestPaint() }
                }

                Connections {
                    target: contentBackground.border
                    function onColorChanged() { arrow.requestPaint() }
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
