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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Project 1.0

import "internal/Migration"

StyledDialogView {
    id: root
    modal: true

    //! TODO: After setting title the accessibility for this dialog on VoiceOver stops working
    //title: qsTrc("project/migration", "Style improvements")

    property string appVersion: ""
    property int migrationType: MigrationType.Unknown

    property bool isApplyLeland: true
    property bool isApplyEdwin: true
    property bool isRemapPercussion: true
    property bool isAskAgain: true

    contentHeight: {
        switch (root.migrationType) {
        case MigrationType.Pre_3_6: return 533 // 517 just fits, then add 16
        case MigrationType.Ver_3_6: return 271 // 255 just fits, then add 16
        case MigrationType.Unknown: return 0
        }
        return 600
    }

    contentWidth: {
        switch (root.migrationType) {
        case MigrationType.Pre_3_6: return 590
        case MigrationType.Ver_3_6: return 491
        case MigrationType.Unknown: return 0
        }
        return 600
    }

    //! NOTE Different dialogs for different migration versions
    onOpened: {
        switch(root.migrationType) {
        case MigrationType.Pre_3_6:
            loader.sourceComponent = migrCompPre362
            break;
        case MigrationType.Ver_3_6:
            isApplyLeland = false
            isApplyEdwin = false
            loader.sourceComponent = migrComp362
            break;
        default:
            console.assert(false, "Wrong migration type!")
        }
    }

    function makeRet(isApply) {
        var ret = {
            errcode: 0,
            value: {
                isApplyMigration: isApply,
                isAskAgain: root.isAskAgain,
                isApplyLeland: root.isApplyLeland,
                isApplyEdwin: root.isApplyEdwin,
                isRemapPercussion: root.isRemapPercussion,
            }
        }

        return ret
    }

    function watchVideo() {
        Qt.openUrlExternally("https://youtu.be/U7dagae87eM")
    }

    ColumnLayout {
        spacing: 0
        anchors.fill: parent

        Loader {
            id: loader

            Layout.fillWidth: true
            Layout.margins: 20

            onLoaded: {
                item.activateNavigation()
            }
        }

        SeparatorLine { }

        RowLayout {
            id: footer

            Layout.leftMargin: 20
            Layout.rightMargin: 20
            Layout.topMargin: 16
            Layout.bottomMargin: 16

            CheckBox {
                id: askAgain
                text: qsTrc("global", "Don’t ask again")
                checked: !root.isAskAgain

                navigation.panel: buttonBox.navigationPanel
                navigation.column: 100

                onClicked: {
                    root.isAskAgain = checked // not `!checked` because the negation is in `checked: !dialog.isAskAgain`
                }
            }

            ButtonBox {
                id: buttonBox
                buttons: [ ButtonBoxModel.Ok ]

                Layout.fillWidth: true

                navigationPanel.section: root.navigationSection
                navigationPanel.order: 1

                onStandardButtonClicked: function(buttonId) {
                    if (buttonId === ButtonBoxModel.Ok) {
                        root.ret = root.makeRet(true)
                        root.hide()
                    }
                }
            }
        }
    }

    //! NOTE for 3.6.2
    Component {
        id: migrComp362

        MigrationContentFor362 {
            appVersion: root.appVersion
            isRemapPercussion: root.isRemapPercussion

            anchors.fill: parent

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 2

            onIsRemapPercussionChangeRequested: function(remapPercussion) {
                root.isRemapPercussion = remapPercussion
            }

            onWatchVideoRequested: {
                root.watchVideo()
            }
        }
    }

    //! NOTE for pre-3.6.2 files
    Component {
        id: migrCompPre362

        MigrationContentForPre362 {
            appVersion: root.appVersion
            isApplyLeland: root.isApplyLeland
            isApplyEdwin: root.isApplyEdwin
            isRemapPercussion: root.isRemapPercussion

            anchors.fill: parent

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 2

            onIsApplyLelandChangeRequested: function(applyLeland) {
                root.isApplyLeland = applyLeland
            }

            onIsApplyEdwinChangeRequested: function(applyEdwin) {
                root.isApplyEdwin = applyEdwin
            }

            onIsRemapPercussionChangeRequested: function(remapPercussion) {
                root.isRemapPercussion = remapPercussion
            }

            onWatchVideoRequested: {
                root.watchVideo()
            }
        }
    }
}

