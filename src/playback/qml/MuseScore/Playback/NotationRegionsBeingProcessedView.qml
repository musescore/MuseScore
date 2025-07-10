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

import MuseScore.Playback 1.0

import "internal"

Loader {
    id: root

    property rect notationViewRect: Qt.rect(0, 0, 0, 0)
    property alias notationViewMatrix: regionsModel.notationViewMatrix

    NotationRegionsBeingProcessedModel {
        id: regionsModel
    }

    active: !regionsModel.isEmpty
    enabled: false // ignore input events

    Component.onCompleted: {
        regionsModel.load()
    }

    onLoaded: {
        item.x = root.notationViewRect.x
        item.y = root.notationViewRect.y
        item.height = root.notationViewRect.height
        item.width = root.notationViewRect.width
        item.model = regionsModel
    }

    sourceComponent: Item {
        property alias model: repeater.model

        clip: true

        Repeater {
            id: repeater

            anchors.fill: parent

            delegate: AudioProcessingProgressBar {
                id: progressBar

                x: model.rect.x
                y: model.rect.y - model.rect.height
                width: Math.max(model.rect.width, progressBar.minWidth)
                height: model.rect.height * 0.7

                progress: model.progress
                backgroundColor: repeater.model.progressBackgroundColor
                textColor: repeater.model.progressTextColor
            }
        }
    }
}
