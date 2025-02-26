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
import QtQuick.Layouts 1.15

import MuseScore.Playback 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0
import MuseScore.CommonScene 1.0

import "internal"

Item {
    id: root

    property bool floating: false

    width: content.width + (floating ? 12 : 0)
    height: content.height

    property NavigationPanel navigationPanel: NavigationPanel {
        id: navPanel
        name: "PlaybackToolBar"
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("playback", "Playback toolbar")
    }

    property alias navigationPanelSection: navPanel.section
    property alias navigationPanelOrder: navPanel.order

    PlaybackToolBarModel {
        id: thePlaybackModel
        isToolbarFloating: root.floating
    }

    Component.onCompleted: {
        thePlaybackModel.load()
    }

    Column {
        id: content

        spacing: 8

        width: childrenRect.width

        enabled: thePlaybackModel.isPlayAllowed

        PlaybackToolBarActions {
            id: playbackActions

            playbackModel: thePlaybackModel
            floating: root.floating

            navPanel: root.navigationPanel
        }

        StyledSlider {
            width: playbackActions.width
            visible: root.floating
            value: thePlaybackModel.playPosition

            onMoved: {
                thePlaybackModel.playPosition = value
            }
        }

        PlaybackSpeedSlider {
            width: playbackActions.width
            visible: root.floating

            playbackModel: thePlaybackModel

            navigationPanel: navPanel
            navigationOrderStart: playbackActions.navigationOrderEnd + 1
        }
    }
}
