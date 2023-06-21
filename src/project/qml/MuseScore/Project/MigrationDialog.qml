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

import "internal/Migration"

StyledDialogView {
    id: dialog

    //! TODO: After setting title the accessibility for this dialog on VoiceOver stops working
    //title: qsTrc("project/migration", "Style improvements")

    property string appVersion: ""
    property int migrationType: MigrationType.Unknown

    property bool isApplyLeland: true
    property bool isApplyEdwin: true
    property bool isAskAgain: true

    contentHeight: {
        switch (dialog.migrationType) {
        case MigrationType.Pre_3_6: return 556
        case MigrationType.Ver_3_6: return 208
        case MigrationType.Ver_4_0: return 208
        case MigrationType.Unknown: return 0
        }
        return 600
    }

    contentWidth:  {
        switch (dialog.migrationType) {
        case MigrationType.Pre_3_6: return 600
        case MigrationType.Ver_3_6: return 480
        case MigrationType.Ver_4_0: return 480
        case MigrationType.Unknown: return 0
        }
        return 600
    }

    modal: true

    //! NOTE Different dialogs for different migration versions
    onOpened: {
        switch(dialog.migrationType) {
        case MigrationType.Pre_3_6:
            loader.sourceComponent = migrComp
            break;
        case MigrationType.Ver_3_6:
        case MigrationType.Ver_4_0:
            isApplyLeland = false
            isApplyEdwin = false
            loader.sourceComponent = noteComp
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
                isAskAgain: dialog.isAskAgain,
                isApplyLeland: dialog.isApplyLeland,
                isApplyEdwin: dialog.isApplyEdwin,
            }
        }

        return ret
    }

    function watchVideo() {
        Qt.openUrlExternally("https://youtu.be/U7dagae87eM")
    }

    Loader {
        id: loader
        anchors.fill: parent

        onLoaded: {
            item.activateNavigation()
        }
    }

    Component {
        id: noteComp

        MigrationContentWithNoticeOnly {
            anchors.fill: parent
            anchors.margins: 16

            appVersion: dialog.appVersion
            isAskAgain: dialog.isAskAgain

            navigationPanel.section: dialog.navigationSection

            onIsAskAgainChangeRequested: function(askAgain) {
                dialog.isAskAgain = askAgain
            }

            onWatchVideoRequested: {
                dialog.watchVideo()
            }

            onAccess: {
                dialog.ret = dialog.makeRet(true)
                dialog.hide()
            }
        }
    }

    //! NOTE for pre-3.6.2 files
    Component {
        id: migrComp

        MigrationContentForPre362 {
            anchors.fill: parent

            appVersion: dialog.appVersion
            isAskAgain: dialog.isAskAgain

            isApplyLeland: dialog.isApplyLeland
            isApplyEdwin: dialog.isApplyEdwin

            navigationSection: dialog.navigationSection

            onIsApplyEdwinChangeRequested: function(applyEdwin) {
                dialog.isApplyEdwin = applyEdwin
            }

            onIsApplyLelandChangeRequested: function(applyLeland) {
                dialog.isApplyLeland = applyLeland
            }

            onIsAskAgainChangeRequested: function(askAgain) {
                dialog.isAskAgain = askAgain
            }

            onWatchVideoRequested: {
                dialog.watchVideo()
            }

            onAccess: {
                dialog.ret = dialog.makeRet(true)
                dialog.hide()
            }
        }
    }
}

