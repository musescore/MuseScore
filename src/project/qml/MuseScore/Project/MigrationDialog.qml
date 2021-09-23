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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Project 1.0

StyledDialogView {
    id: dialog

    title: "" // will be set when open

    property string version: "?"
    property bool applyLeland: false
    property bool applyEdwin: false
    property bool rememberChoice: false

    contentWidth: 600
    contentHeight: 600

    modal: true

    function makeRet(isApply) {
        var ret = {
            errcode: 0,
            value: {
                apply: isApply,
                rememberChoice: dialog.rememberChoice,
                applyLeland: dialog.applyLeland,
                applyEdwin: dialog.applyEdwin
            }
        }

        return ret
    }

    Item {
        id: content
        anchors.fill: parent
        anchors.margins: 16

        StyledTextLabel {
            id: headerLabel
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 36

            font.bold: true
            font.pixelSize: 20
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter

            text: qsTrc("project", "Would you like to try our improved score style?")
        }

        Image {
            id: imageItem
            anchors.top: headerLabel.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 200
            fillMode: Image.PreserveAspectFit
            source: "migration.png"
        }

        CheckBox {
            id: lelandOption
            anchors.top: imageItem.bottom
            anchors.left: parent.left
            anchors.topMargin: 24
            text: qsTrc("project", "Our new professional notation font, Leland")
            checked: dialog.applyLeland
            onClicked: dialog.applyLeland = !dialog.applyLeland
        }

        CheckBox {
            id: edwinOption
            anchors.top: lelandOption.bottom
            anchors.left: parent.left
            anchors.topMargin: 16
            text: qsTrc("project", "Our improved text font, Edwin")
            checked: dialog.applyEdwin
            onClicked: dialog.applyEdwin = !dialog.applyEdwin
        }

        StyledTextLabel {
            id: versionLabel
            anchors.top: edwinOption.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 32
            font.pixelSize: 14
            horizontalAlignment: Qt.AlignLeft
            verticalAlignment: Qt.AlignVCenter

            text: qsTrc("project", "Since this file was created in MuseScore %1, some layout changes may occur.").arg(dialog.version)
        }

        StyledTextLabel {
            id: linkLabel
            anchors.top: versionLabel.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 16
            font.pixelSize: 14
            horizontalAlignment: Qt.AlignLeft
            verticalAlignment: Qt.AlignVCenter

            text: "<a href=\"%1\">%2</a>"
            .arg(Qt.locale().name === "zh_CN" ? "https://www.bilibili.com/video/BV1FT4y1K7UM" : "https://youtu.be/qLR40BGNy68")
            .arg(qsTr("Watch our release video to learn more"))
        }

        Item {
            id: footer

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 112

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                height: 2
                color: ui.theme.buttonColor
            }

            CheckBox {
                id: askAgain
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.topMargin: 16
                text: qsTrc("project", "Remember my choice and don't ask again")
                checked: dialog.rememberChoice
                onClicked: dialog.rememberChoice = !dialog.rememberChoice
            }

            FlatButton {
                anchors.right: applyBtn.left
                anchors.bottom: parent.bottom
                anchors.margins: 16
                text: qsTrc("project", "Keep old style")
                onClicked: {
                    dialog.ret = dialog.makeRet(false)
                    dialog.hide()
                }
            }

            FlatButton {
                id: applyBtn
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 16
                text: qsTrc("project", "Apply new style")
                onClicked: {
                    dialog.ret = dialog.makeRet(true)
                    dialog.hide()
                }
            }
        }
    }
}
