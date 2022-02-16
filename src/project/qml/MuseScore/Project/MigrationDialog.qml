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

    property string appVersion: ""
    property int migrationType: MigrationType.Unknown

    property bool isApplyLeland: true
    property bool isApplyEdwin: true
    property bool isApplyAutoSpacing: true
    property bool isAskAgain: true

    contentHeight: {
        switch (dialog.migrationType) {
        case MigrationType.Pre300: return 588
        case MigrationType.Post300AndPre362: return 556
        case MigrationType.Ver362: return 160
        case MigrationType.Unknown: return 0
        }
        return 600
    }

    contentWidth:  {
        switch (dialog.migrationType) {
        case MigrationType.Pre300: return 600
        case MigrationType.Post300AndPre362: return 600
        case MigrationType.Ver362: return 480
        case MigrationType.Unknown: return 0
        }
        return 600
    }

    modal: true

    //! NOTE Can be set three a fixed version, for each version different dialog content
    onOpened: {

        switch(dialog.migrationType) {
        case MigrationType.Pre300:
            loader.sourceComponent = migrComp
            break;
        case MigrationType.Post300AndPre362:
            isApplyAutoSpacing = false
            loader.sourceComponent = migrComp
            break;
        case MigrationType.Ver362:
            isApplyLeland = false
            isApplyEdwin = false
            isApplyAutoSpacing = false
            loader.sourceComponent = noteComp
            break;
        default: {
            console.assert(false, "Wrong migration type!")
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

                    font: ui.theme.tabBoldFont

                    horizontalAlignment: Qt.AlignLeft
                    verticalAlignment: Qt.AlignVCenter

                    text: qsTrc("project", "This file was created in MuseScore %1").arg(dialog.appVersion)
                }

                StyledTextLabel {
                    id: headerSubtitle
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 24

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
                    onClicked: dialog.watchVideo()
                }

                FlatButton {
                    id: applyBtn
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTrc("global", "OK")
                    onClicked: {
                        dialog.ret = dialog.makeRet(true)
                        dialog.hide()
                    }
                }
            }
        }
    }

    //! NOTE for pre-3.6.2 files
    Component {
        id: migrComp

        Item {
            id: content

            anchors.fill: parent

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

                    font: ui.theme.tabBoldFont

                    horizontalAlignment: Qt.AlignLeft
                    verticalAlignment: Qt.AlignVCenter

                    text: qsTrc("project", "This file was created in MuseScore %1").arg(dialog.appVersion)
                }

                StyledTextLabel {
                    id: headerSubtitle
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 24

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
                    visible: dialog.migrationType === MigrationType.Pre300
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

                    horizontalAlignment: Qt.AlignLeft
                    verticalAlignment: Qt.AlignVCenter

                    text: qsTrc("project", "Please note: score layouts will be affected by improvements to MuseScore 4")
                }

                FlatButton {
                    id: watchVideo

                    text: qsTrc("project", "Watch video")
                    onClicked: dialog.watchVideo()
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
                    text: qsTrc("global", "OK")
                    onClicked: {
                        dialog.ret = dialog.makeRet(true)
                        dialog.hide()
                    }
                }
            }
        }
    }
}

