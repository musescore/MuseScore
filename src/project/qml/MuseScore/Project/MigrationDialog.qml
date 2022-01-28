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

    title: qsTrc("project", "Style Improvements")

    property int version: 362 // can be: 362, 323, 225
    property bool isApplyLeland: true
    property bool isApplyEdwin: true
    property bool isApplyAutoSpacing: true
    property bool isAskAgain: true

    contentHeight: {
        switch (dialog.version) {
        case 225: return 588
        case 323: return 556
        case 362: return 160
        }
        return 600
    }

    contentWidth:  {
        switch (dialog.version) {
        case 225: return 600
        case 323: return 600
        case 362: return 480
        }
        return 600
    }

    modal: true

    //! NOTE Can be set three a fixed version, for each version different dialog content
    onOpened: {

        switch(dialog.version) {
        case 225: {
            loader.sourceComponent = migrComp
            loader.item.version = dialog.version
        } break;
        case 323: {
            loader.sourceComponent = migrComp
            loader.item.version = dialog.version
        } break;
        case 362: {
            loader.sourceComponent = noteComp
        } break;
        default: {
            console.assert(false, "Version must be 225, 323, 362")
        }
        }
    }

    function makeRet(isApply) {
        var ret = {
            errcode: 0,
            value: {
                isApplyMigration: isApply,
                isAskAgain: dialog.isAskAgain,
                isApplyLeland: dialog.isApplyLeland,
                isApplyEdwin: dialog.isApplyEdwin,
                isApplyAutoSpacing: dialog.isApplyAutoSpacing
            }
        }

        return ret
    }

    function watchVideo() {
        var link = Qt.locale().name === "zh_CN" ? "https://www.bilibili.com/video/BV1FT4y1K7UM" : "https://youtu.be/qLR40BGNy68"
        Qt.openUrlExternally(link)
    }

    Loader {
        id: loader
        anchors.fill: parent
    }

    //! NOTE for 3.6.2
    Component {
        id: noteComp

        Item {
            id: content

            property int version: -1

            anchors.fill: parent

            Column {
                id: mainContent
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16

                StyledTextLabel {
                    id: headerTitle
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 32

                    font.weight: Font.DemiBold
                    font.pixelSize: 16
                    horizontalAlignment: Qt.AlignLeft
                    verticalAlignment: Qt.AlignVCenter

                    text: qsTrc("project", "This file was created in MuseScore %1").arg("3.6.2")
                }

                StyledTextLabel {
                    id: headerSubtitle
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 24

                    font.weight: Font.Medium
                    font.pixelSize: 12
                    horizontalAlignment: Qt.AlignLeft
                    verticalAlignment: Qt.AlignVCenter
                    wrapMode: Text.WordWrap
                    elide: Text.ElideNone

                    text: qsTrc("project", "Please note that the appearance of your score will change due to improvements we have made to default settings for beaming, ties, slurs, system objects and horizontal spacing.")
                }
            }

            Item {
                id: footer

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                height: 56

                FlatButton {
                    id: watchVideo
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTrc("project", "Watch video about changes")
                    onClicked:  dialog.watchVideo()
                }

                FlatButton {
                    id: applyBtn
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTrc("global", "Ok")
                    onClicked: {
                        dialog.ret = dialog.makeRet(true)
                        dialog.hide()
                    }
                }
            }
        }
    }

    //! NOTE for 2.2.5 and 3.2.3
    Component {
        id: migrComp

        Item {
            id: content

            property int version: -1

            anchors.fill: parent

            function userVersion() {
                switch (content.version) {
                case 225: return "2.2.5"
                case 323: return "3.2.3"
                }
                return ""
            }

            Column {
                id: mainContent
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 16
                height: childrenRect.height

                StyledTextLabel {
                    id: headerTitle
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 32

                    font.weight: Font.DemiBold
                    font.pixelSize: 16
                    horizontalAlignment: Qt.AlignLeft
                    verticalAlignment: Qt.AlignVCenter

                    text: qsTrc("project", "This file was created in MuseScore %1").arg(content.userVersion())
                }

                StyledTextLabel {
                    id: headerSubtitle
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 24

                    font.weight: Font.Medium
                    font.pixelSize: 12
                    horizontalAlignment: Qt.AlignLeft
                    verticalAlignment: Qt.AlignVCenter

                    text: qsTrc("project", "Select the engraving improvements you would like to apply to your score")
                }

                Item {
                    id: imageItem
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 264

                    Image {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        height: 200
                        fillMode: Image.PreserveAspectFit
                        source: "migration.png"
                    }
                }

                CheckBox {
                    id: lelandOption
                    anchors.left: parent.left
                    height: 32
                    text: qsTrc("project", "Our new notation font, Leland")
                    checked: dialog.isApplyLeland
                    onClicked: dialog.isApplyLeland = !dialog.isApplyLeland
                }

                CheckBox {
                    id: edwinOption
                    anchors.left: parent.left
                    height: 32
                    text: qsTrc("project", "Our new text font, Edwin")
                    checked: dialog.isApplyEdwin
                    onClicked: dialog.isApplyEdwin = !dialog.isApplyEdwin
                }

                CheckBox {
                    id: spacingOption
                    anchors.left: parent.left
                    height: 32
                    text: qsTrc("project", "Automatic spacing (introduced in MuseScore 3.0)")
                    visible: content.version == 225
                    checked: dialog.isApplyAutoSpacing
                    onClicked: dialog.isApplyAutoSpacing = !dialog.isApplyAutoSpacing
                }

                Item {
                    width: 1
                    height: 16
                }

                StyledTextLabel {
                    id: noteLabel
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 32

                    font.weight: Font.Medium
                    font.pixelSize: 12
                    horizontalAlignment: Qt.AlignLeft
                    verticalAlignment: Qt.AlignVCenter

                    text: qsTrc("project", "Please note: score layouts will be affected by improvements to MuseScore 4")
                }

                FlatButton {
                    id: watchVideo

                    text: qsTrc("project", "Watch video")
                    onClicked:  dialog.watchVideo()
                }
            }

            Item {
                id: footer

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                height: 56

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: -16
                    height: 2
                    color: ui.theme.buttonColor
                }

                CheckBox {
                    id: askAgain
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTrc("project", "Don't ask again")
                    checked: !dialog.isAskAgain
                    onClicked: dialog.isAskAgain = !dialog.isAskAgain
                }

                FlatButton {
                    id: applyBtn
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTrc("global", "Ok")
                    onClicked: {
                        dialog.ret = dialog.makeRet(true)
                        dialog.hide()
                    }
                }
            }
        }
    }
}

