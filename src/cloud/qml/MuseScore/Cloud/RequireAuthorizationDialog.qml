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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Cloud 1.0

StyledDialogView {
    id: root

    property alias text: content.text
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
            { "buttonId": ButtonBoxModel.CustomButton + 1, "text": qsTrc("cloud", "Create account"), "role": ButtonBoxModel.ApplyRole, "isAccent": false, "isLeftSide": false },
            { "buttonId": ButtonBoxModel.CustomButton + 2, "text": qsTrc("cloud", "Login"), "role": ButtonBoxModel.ApplyRole, "isAccent": false, "isLeftSide": false }
        ]

        onClicked: function(buttonId, showAgain) {
            switch (buttonId) {
            case ButtonBoxModel.Cancel:
                root.hide()
                return
            case ButtonBoxModel.CustomButton + 1:
                cloudsModel.createAccount(root.cloudCode)
                return
            case ButtonBoxModel.CustomButton + 2:
                cloudsModel.signIn(root.cloudCode)
                return
            }
        }
    }
}

