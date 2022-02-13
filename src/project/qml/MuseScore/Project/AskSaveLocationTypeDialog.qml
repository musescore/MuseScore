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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Project 1.0

import "internal/SaveToCloud"

StyledDialogView {
    id: root

    contentHeight: 600
    contentWidth: 900

    objectName: "AskSaveLocationTypeDialog"

    property bool askAgain: true

    function done(saveLocationType) {
        root.ret = {
            errcode: 0,
            value: {
                askAgain: root.askAgain,
                saveLocationType: saveLocationType
            }
        }

        root.hide()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 24

        StyledTextLabel {
            Layout.fillWidth: true
            text: qsTrc("project", "How would you like to save?")
            font: ui.theme.headerBoldFont
        }

        RowLayout {
            spacing: 24

            SaveLocationOption {
                title: qsTrc("project", "To the Cloud (free)")
                description: qsTrc("project", "Files are saved privately on your own personal account. \
You can share drafts with others and publish your finished scores publicly too.")
                buttonText: qsTrc("project", "Save to the cloud")

                imageSource: "internal/SaveToCloud/images/Cloud.png"

                onButtonClicked: {
                    root.done(SaveLocationType.Cloud)
                }
            }

            SaveLocationOption {
                title: qsTrc("project", "On your computer")
                description: qsTrc("project", "If you prefer to save your files on your computer, you can do that here.")
                buttonText: qsTrc("project", "Save to computer")

                imageSource: "internal/SaveToCloud/images/Laptop.png"

                onButtonClicked: {
                    root.done(SaveLocationType.Local)
                }
            }
        }

        SeparatorLine { Layout.margins: -24 }

        CheckBox {
            id: dontAskAgainCheckbox
            width: parent.width
            text: qsTrc("project", "Don’t show again")
            checked: !root.askAgain

            onClicked: {
                root.askAgain = !root.askAgain
            }
        }
    }
}
