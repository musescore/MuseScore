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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Update 1.0

import "internal"

StyledDialogView {
    id: root

    property string version: ""
    property string mode: ""

    contentWidth: 400
    contentHeight: 56

    margins: 24

    UpdateModel {
        id: updateModel

        onFinished: function(errorCode, installerPath) {
            root.ret = { errcode: errorCode, value: installerPath }
            root.hide()
        }
    }

    Component.onCompleted: {
        updateModel.load(root.mode)
    }

    onNavigationActivateRequested: {
        progressBar.navigation.requestActive()
    }

    ColumnLayout {
        id: content

        anchors.fill: parent
        spacing: 20

        StyledTextLabel {
            Layout.alignment: Qt.AlignTop

            text: updateModel.progressTitle
            font: ui.theme.largeBodyBoldFont
        }

        ProgressBar {
            id: progressBar

            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true
            Layout.preferredHeight: 4

            from: 0
            to: updateModel.totalProgress
            value: updateModel.currentProgress
        }
    }
}
