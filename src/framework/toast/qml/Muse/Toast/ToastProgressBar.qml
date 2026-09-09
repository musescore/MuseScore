/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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
import QtQuick.Controls
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

Item {
    id: root

    height: mainContainer.height

    property int progress: 0
    property int timeElapsed: 0

    property int contentSpacing: 8
    property int contentMargins: 0
    property int messagePixelSize: 14
    property int progressBarHeight: 8

    onTimeElapsedChanged: {
        progressTextHelper.updateText()
    }

    onProgressChanged: {
        progressTextHelper.updateText()
    }

    QtObject {
        id: progressTextHelper

        property string text: ""

        function updateText() {
            if (root.progress <= 0 || root.timeElapsed <= 0) {
                text = ""
                return
            }

            const SECONDS_IN_HOUR = 3600
            const SECONDS_IN_MINUTE = 60

            const estimatedTotalTime = root.timeElapsed / (root.progress / 100.0)
            const remainingSeconds = Math.max(0, estimatedTotalTime - root.timeElapsed)

            if (remainingSeconds >= SECONDS_IN_HOUR) {
                const hours = Math.round(remainingSeconds / SECONDS_IN_HOUR)
                text = qsTrc("toast", "%n hour(s) remaining", "amount of time left until the requested operation is finished", hours)
            } else if (remainingSeconds >= SECONDS_IN_MINUTE) {
                const minutes = Math.round(remainingSeconds / SECONDS_IN_MINUTE)
                text = qsTrc("toast", "%n minute(s) remaining", "amount of time left until the requested operation is finished", minutes)
            } else {
                const seconds = Math.round(remainingSeconds)
                text = qsTrc("toast", "%n second(s) remaining", "amount of time left until the requested operation is finished", seconds)
            }
        }
    }

    Column {
        id: mainContainer

        anchors.left: parent.left
        anchors.right: parent.right

        height: progressBar.height + progressText.height + root.contentSpacing

        anchors.margins: root.contentMargins

        spacing: root.contentSpacing

        ProgressBar {
            id: progressBar

            width: parent.width
            height: root.progressBarHeight
            value: root.progress / 100.0
        }

        Item {
            id: progressText

            width: parent.width
            height: leftLabel.height

            StyledTextLabel {
                id: leftLabel

                anchors.left: parent.left
                horizontalAlignment: Text.AlignLeft

                font.pixelSize: root.messagePixelSize

                text: progressTextHelper.text
            }

            StyledTextLabel {
                id: rightLabel

                anchors.right: parent.right
                horizontalAlignment: Text.AlignRight

                font.pixelSize: root.messagePixelSize

                text: Math.min(Math.max(root.progress, 0), 100) + "%"
            }
        }
    }
}
