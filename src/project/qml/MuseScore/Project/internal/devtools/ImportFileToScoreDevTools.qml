/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.Project

Rectangle {
    color: ui.theme.backgroundSecondaryColor

    ImportFileToScoreDevToolsModel {
        id: model
    }

    Component.onCompleted: {
        model.init()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12

        spacing: 12

        RowLayout {
            spacing: 8

            FlatButton {
                text: "Select && import files"
                onClicked: model.selectAndImportFiles()
            }

            FlatButton {
                text: "Check import in progress"
                onClicked: model.checkImportInProgress()
            }

            FlatButton {
                text: "Clear log"
                onClicked: model.clearLog()
            }
        }

        StyledTextLabel {
            text: "Log:"
        }

        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true

            contentWidth: width
            contentHeight: logLabel.implicitHeight
            clip: true

            StyledTextLabel {
                id: logLabel

                width: parent.width

                text: model.log
                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.Wrap
            }
        }
    }
}
