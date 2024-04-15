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

Item {
    id: root

    signal started()
    signal finished()

    PlaybackLoadingModel {
        id: model

        onStarted: {
            root.started()
        }

        onFinished: {
            root.finished()
        }
    }

    Component.onCompleted: {
        model.load()
    }

    RowLayout {
        anchors.fill: parent
        spacing: 12

        ProgressBar {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: 160
            Layout.preferredHeight: 14

            from: 0
            to: model.totalProgress
            value: model.currentProgress
        }

        StyledTextLabel {
            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true

            text: model.progressTitle
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
        }
    }
}
