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
import QtQuick.Controls 2.15 as Controls

import MuseScore.UiComponents 1.0
import MuseScore.Project 1.0

StyledDialogView {
    id: root

    contentWidth: 240
    contentHeight: 100
    margins: 12

    modal: true

    ExportProgressModel {
        id: model

        onExportFinished: {
            root.close()
        }
    }

    Component.onCompleted: {
        model.load()
    }

    ColumnLayout {
        anchors.fill: parent

        spacing: 8

        StyledTextLabel {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            text: qsTrc("project", "Exporting…")
        }

        Controls.ProgressBar {
            Layout.fillWidth: true
            Layout.preferredHeight: 30

            indeterminate: true
        }

        /* TODO
        ProgressBar {
            Layout.fillWidth: true
            Layout.preferredHeight: 30

            from: 0
            to: 100
            value: model.progress

            progressStatus: model.progress + "%"
        }

        FlatButton {
            Layout.alignment: Qt.AlignRight

            text: qsTrc("global", "Cancel")

            onClicked: {
                model.cancel()
                root.close()
            }
        }*/
    }
}
