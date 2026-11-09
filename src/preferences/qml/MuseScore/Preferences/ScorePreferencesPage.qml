/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

import Muse.UiComponents
import MuseScore.Preferences

import "internal"

PreferencesPage {
    id: root

    ScorePreferencesModel {
        id: scorePreferencesModel
    }

    Component.onCompleted: {
        scorePreferencesModel.load()
    }

    Column {
        width: parent.width
        spacing: root.sectionsSpacing

        DefaultStyleSection {
            width: parent.width

            model: scorePreferencesModel

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 1
        }

        SeparatorLine { }

        PlaybackCursorsSection {
            width: parent.width

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 2

            isShowingPlaybackCueInNormalMode: scorePreferencesModel.showPlaybackCueInNormalMode
            isShowingPlaybackCursorInNormalMode: scorePreferencesModel.showPlaybackCursorInNormalMode

            onShowPlaybackCueInNormalModeChangeRequested: function(show) {
                scorePreferencesModel.showPlaybackCueInNormalMode = show
            }

            onShowPlaybackCursorInNormalModeChangeRequested: function(show) {
                scorePreferencesModel.showPlaybackCursorInNormalMode = show
            }

            playbackCueColor: scorePreferencesModel.playbackCueColor
            playbackCursorColor: scorePreferencesModel.playbackCursorColor

            onPlaybackCueColorChangeRequested: function(newColor) {
                scorePreferencesModel.playbackCueColor = newColor
            }

            onPlaybackCursorColorChangeRequested: function(newColor) {
                scorePreferencesModel.playbackCursorColor = newColor
            }
        }
    }
}
