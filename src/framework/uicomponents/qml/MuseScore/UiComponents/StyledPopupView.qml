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
import QtGraphicalEffects 1.0

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

PopupView {
    id: root

    default property alias contentData: contentBody.data

    property alias background: contentBackground

    property alias width: rootContainer.width
    property alias height: rootContainer.height

    property int padding: 16
    property int margins: 16

    property int contentWidth: 240
    property int contentHeight: contentBody.childrenRect.height

    property bool opensUpward: false
    property var arrowX: width / 2

    property bool animationEnabled: true

    property alias navigation: keynavPanel
    property bool isDoActiveParentOnClose: true

    closePolicy: PopupView.CloseOnPressOutsideParent

    x: parent.width / 2
    y: opensUpward ? (-height) : (parent.height - root.padding)

    property NavigationPopupPanel keynavPanel: NavigationPopupPanel {
        id: keynavPanel
        enabled: root.isOpened
        order: {
            var pctrl = keynavPanel.parentControl;
            if (pctrl) {
                if (pctrl.panel) {
                    return pctrl.panel.order + 1
                }
            }
            return -1
        }

        section: {
            var pctrl = keynavPanel.parentControl;
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
                rootContainer.focus = true
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
        if (root.isDoActiveParentOnClose && keynavPanel.parentControl) {
            keynavPanel.parentControl.forceActive()
        }
    }

    contentItem: FocusScope {
        id: rootContainer
        width: contentContainer.width + root.padding * 2
        height: contentContainer.height + root.padding * 2

        implicitWidth: contentContainer.implicitWidth + root.padding * 2
        implicitHeight: contentContainer.implicitHeight + root.padding * 2

        Item {
            id: contentContainer
            x: root.padding
            y: root.padding
            width: contentBody.width + root.margins * 2
            height: contentBody.height + root.margins * 2

            implicitWidth: contentBody.implicitWidth + root.margins * 2
            implicitHeight: contentBody.implicitHeight + root.margins * 2

            transformOrigin: Item.Center

            Rectangle {
                id: contentBackground
                anchors.fill: parent
                color: ui.theme.backgroundPrimaryColor
                border.width: 1
                border.color: ui.theme.strokeColor
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
                PropertyChanges { target: contentContainer; scale: 0.7; opacity: 0.5 }
            }
        ]

        transitions: Transition {
            NumberAnimation { properties: "scale, opacity"; easing.type: Easing.OutQuint; duration: root.animationEnabled ? 300 : 0 }
        }
    }
}
