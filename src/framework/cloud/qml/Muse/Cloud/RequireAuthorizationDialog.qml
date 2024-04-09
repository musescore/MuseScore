/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Cloud 1.0

StyledDialogView {
    id: root

    property alias text: content.text
    property bool publishingScore: true
    property string cloudCode: ""

    contentWidth: content.implicitWidth
    contentHeight: content.implicitHeight

    margins: 16

    CloudsModel {
        id: cloudsModel

        onUserAuthorizedChanged: {
            if (userAuthorized) {
                root.accept()
            }
        }
    }

    onNavigationActivateRequested: {
        content.focusOnFirst()
    }

    onAccessibilityActivateRequested: {
        content.readInfo()
    }

    Component.onCompleted: {
        cloudsModel.load()
    }

    StandardDialogPanel {
        id: content
        anchors.fill: parent

        navigation.section: root.navigationSection

        title: qsTrc("cloud", "You are not signed in")

        customButtons: [
            { "buttonId": ButtonBoxModel.Cancel, "text": qsTrc("global", "Cancel"), "role": ButtonBoxModel.RejectRole, "isAccent": false, "isLeftSide": false },

            { "buttonId": ButtonBoxModel.CustomButton + 1,
              "text": publishingScore ? qsTrc("project/save", "Save to computer") : qsTrc("cloud", "Create account"),
              "role": ButtonBoxModel.ApplyRole, "isAccent": false, "isLeftSide": false },

            { "buttonId": ButtonBoxModel.CustomButton + 2, "text": qsTrc("cloud", "Log in"), "role": ButtonBoxModel.ApplyRole, "isAccent": false, "isLeftSide": false }
        ]

        onClicked: function(buttonId, showAgain) {
            switch (buttonId) {
            case ButtonBoxModel.Cancel:
                root.hide()
                return
            case ButtonBoxModel.CustomButton + 1:
                if (publishingScore) {
                    root.ret = {
                        errcode: 0,
                        value: SaveToCloudResponse.SaveLocallyInstead
                    }
                    root.hide()
                    return
                }
                cloudsModel.createAccount(root.cloudCode)
                return
            case ButtonBoxModel.CustomButton + 2:
                cloudsModel.signIn(root.cloudCode)
                return
            }
        }
    }
}

