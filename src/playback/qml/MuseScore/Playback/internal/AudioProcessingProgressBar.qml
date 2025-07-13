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

import QtQuick 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

Item {
    id: root

    property int progress: 0 // 0-100%
    property color backgroundColor
    property color textColor

    readonly property real minWidth: iconLabel.width * 2 + prv.sideMargin

    QtObject {
        id: prv

        readonly property real sideMargin: iconLabel.width / 3
        readonly property real spacing: iconLabel.width / 2
        readonly property real freeSpace: background.width - iconLabel.width - prv.sideMargin - prv.spacing
    }

    Rectangle {
        id: background

        anchors.fill: parent

        radius: background.height * 0.7
        color: root.backgroundColor

        SequentialAnimation on opacity {
            loops: Animation.Infinite

            NumberAnimation {
                from: 0.3
                to: 0.6
                duration: 1000
                easing.type: Easing.InOutQuad
            }

            NumberAnimation {
                from: 0.6
                to: 0.3
                duration: 1000
                easing.type: Easing.InOutQuad
            }
        }
    }

    StyledIconLabel {
        id: iconLabel

        anchors.left: parent.left
        anchors.leftMargin: prv.sideMargin
        anchors.verticalCenter: parent.verticalCenter

        z: background.z + 1

        opacity: 1
        color: root.textColor

        horizontalAlignment: Text.AlignLeft
        font.pixelSize: background.height * 0.8

        iconCode: IconCode.AUDIO

        states: [
            State {
                name: "CENTERED"
                when: textLabel.empty

                AnchorChanges {
                    target: iconLabel
                    anchors.left: undefined
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        ]
    }

    StyledTextLabel {
        id: textLabel

        //: %1 will be replaced with percentage (i.e. if it is replaced with 50, the string will read "Processing 50%…")
        readonly property string longText: qsTrc("global", "Processing %1%…")
        readonly property string longTextNoPercent: qsTrc("global", "Processing…")
        //: %1 will be replaced with percentage (i.e. if it is replaced with 50, the string will read "50%")
        readonly property string shortText: qsTrc("global", "%1%")
        readonly property bool empty: textLabel.text.length === 0

        anchors.left: iconLabel.right
        anchors.leftMargin: prv.spacing
        anchors.verticalCenter: parent.verticalCenter

        z: background.z + 1

        opacity: 1
        color: root.textColor

        horizontalAlignment: Text.AlignLeft
        font.pixelSize: background.height * 0.6

        text: {
            if (root.progress <= 0) {
                if (longTextNoPercentMetrics.width < prv.freeSpace) {
                    return textLabel.longTextNoPercent
                }

                return ""
            }

            if (longTextMetrics.width < prv.freeSpace) {
                return textLabel.longText.arg(root.progress)
            } else if (shortTextMetrics.width < prv.freeSpace) {
                return textLabel.shortText.arg(root.progress)
            }

            return ""
        }

        TextMetrics {
            id: longTextMetrics

            font: textLabel.font
            text: textLabel.longText.arg(100) // make sure 100% fits
        }

        TextMetrics {
            id: longTextNoPercentMetrics

            font: textLabel.font
            text: textLabel.longTextNoPercent
        }

        TextMetrics {
            id: shortTextMetrics

            font: textLabel.font
            text: textLabel.shortText.arg(100) // make sure 100% fits
        }
    }
}
