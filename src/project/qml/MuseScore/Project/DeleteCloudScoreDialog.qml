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

import Muse.Ui
import Muse.UiComponents

StyledDialogView {
    id: root

    property string scoreName: ""

    objectName: "DeleteCloudScoreDialog"

    contentWidth: 480
    contentHeight: content.implicitHeight
    margins: 16

    modal: true

    function done(agreesToDelete) {
        root.ret = { errcode: 0, value: { "agreesToDelete": agreesToDelete } }
        root.hide()
    }

    onNavigationActivateRequested: {
        confirmCheckBox.navigation.requestActive()
    }

    Column {
        id: content

        width: parent.width
        spacing: 24

        Row {
            id: mainRow

            width: parent.width
            spacing: 27

            StyledIconLabel {
                id: icon

                width: 48
                height: 48

                font.pixelSize: 48
                iconCode: IconCode.WARNING
            }

            Column {
                width: mainRow.width - icon.width - mainRow.spacing
                spacing: 18

                StyledTextLabel {
                    id: titleLabel

                    width: parent.width

                    text: root.scoreName !== ""
                          //: %1 is the name of the score that is about to be deleted
                          ? qsTrc("project/cloud", "Are you sure you want to delete “%1”?").arg(root.scoreName)
                          : qsTrc("project/cloud", "Are you sure you want to delete this score?")
                    textFormat: Text.PlainText
                    font: ui.theme.largeBodyBoldFont
                    horizontalAlignment: Text.AlignLeft
                    wrapMode: Text.Wrap
                }

                CheckBox {
                    id: confirmCheckBox

                    width: parent.width

                    text: qsTrc("project/cloud", "I understand that deleting is permanent and can’t be undone.")
                    checked: false

                    navigation.panel: NavigationPanel {
                        name: "DeleteCloudScoreConfirmation"
                        section: root.navigationSection
                        order: 1
                    }
                    navigation.accessible.name: titleLabel.text + " " + confirmCheckBox.text

                    onClicked: {
                        confirmCheckBox.checked = !confirmCheckBox.checked
                    }
                }
            }
        }

        ButtonBox {
            id: buttonBox

            width: parent.width

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 2

            FlatButton {
                text: qsTrc("global", "Go back")

                buttonRole: ButtonBoxModel.RejectRole
                buttonId: ButtonBoxModel.CustomButton + 1

                onClicked: root.done(false)
            }

            FlatButton {
                text: qsTrc("project/cloud", "Delete score")
                accentButton: true
                enabled: confirmCheckBox.checked

                buttonRole: ButtonBoxModel.AcceptRole
                buttonId: ButtonBoxModel.CustomButton + 2

                onClicked: root.done(true)
            }
        }
    }
}
