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
import QtQuick.Layouts 1.15

import MuseScore.Playback 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.CommonScene 1.0

import "internal"

Item {
    id: root

    property alias navigation: navPanel
    property bool floating: false

    width: content.width
    height: content.height

    NavigationPanel {
        id: navPanel
        name: "PlaybackToolBar"
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("playback", "Playback toolbar")
    }

    PlaybackToolBarModel {
        id: playbackModel
        isToolbarFloating: root.floating
    }

    Component.onCompleted: {
        playbackModel.load()
    }

    Column {
        id: content

        spacing: 8

        width: childrenRect.width

        enabled: playbackModel.isPlayAllowed

        PlaybackToolBarActions {
            id: playbackActions

            playbackModel: playbackModel
            floating: root.floating

            navPanel: root.navigation

            onShortcutChanged: {
                playbackModel.load()
            }
        }

        StyledSlider {
            width: playbackActions.width - 12
            visible: root.floating
            value: playbackModel.playPosition

            onMoved: {
                playbackModel.playPosition = value
            }
        }

        TempoSlider {
            width: playbackActions.width - 12
            visible: root.floating
            value: playbackModel.tempoMultiplier

            onMoved: function(newValue) {
                playbackModel.tempoMultiplier = newValue
            }
        }
    }
}
