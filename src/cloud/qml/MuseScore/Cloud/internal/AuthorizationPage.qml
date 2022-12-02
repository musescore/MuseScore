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
import QtQuick 2.7
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Cloud 1.0

FocusScope {
    id: root

    property NavigationSection navigationSection: null

    signal createAccountRequested()
    signal signInRequested()

    Rectangle {
        anchors.fill: parent
        color: ui.theme.backgroundSecondaryColor
    }

    QtObject {
        id: privateProperties

        readonly property int sideMargin: 46
        readonly property int buttonWidth: 160
    }

    NavigationPanel {
        id: navPanel
        name: "AuthorizationPanel"
        direction: NavigationPanel.Horizontal
        section: root.navigationSection
        accessible.name: pageTitle.text

        onActiveChanged: function(active) {
            if (active) {
                createNewAccount.navigation.requestActive()
                description.readInfo()
            } else {
                description.resetFocusOnInfo()
                createNewAccount.accessible.ignored = true
            }
        }
    }

    StyledTextLabel {
        id: pageTitle

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: privateProperties.sideMargin
        anchors.leftMargin: privateProperties.sideMargin

        text: qsTrc("cloud", "Account")

        font: ui.theme.titleBoldFont
    }

    AccountBenefitsDescription {
        id: description
        anchors.top: pageTitle.bottom
        anchors.topMargin: 48
        anchors.left: parent.left
        anchors.leftMargin: privateProperties.sideMargin

        navigationPanel: navPanel
        activeButtonName: createNewAccount.text
    }

    Rectangle {
        anchors.bottom: parent.bottom

        height: 100
        width: parent.width

        color: ui.theme.backgroundSecondaryColor

        RowLayout {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: privateProperties.sideMargin
            anchors.right: parent.right
            anchors.rightMargin: privateProperties.sideMargin

            spacing: 22

            FlatButton {
                Layout.alignment: Qt.AlignLeft

                minWidth: privateProperties.buttonWidth
                text: qsTrc("cloud", "Learn more")

                navigation.name: "LearnMore"
                navigation.panel: navPanel
                navigation.column: 2

                onClicked: {
                    Qt.openUrlExternally("https://youtu.be/6LP4U_BF23w")
                }
            }

            Row {
                Layout.alignment: Qt.AlignRight

                spacing: 22

                FlatButton {
                    minWidth: privateProperties.buttonWidth
                    text: qsTrc("cloud", "Sign in")

                    navigation.name: "SignIn"
                    navigation.panel: navPanel
                    navigation.column: 3

                    onClicked: {
                        root.signInRequested()
                    }
                }

                FlatButton {
                    id: createNewAccount
                    minWidth: privateProperties.buttonWidth
                    text: qsTrc("cloud", "Create new account")

                    accentButton: true

                    navigation.name: "CreateNewAccount"
                    navigation.panel: navPanel
                    navigation.column: 1
                    navigation.accessible.ignored: true
                    navigation.onActiveChanged: {
                        if (!navigation.active) {
                            accessible.ignored = false
                            description.resetFocusOnInfo()
                        }
                    }

                    onClicked: {
                        root.createAccountRequested()
                    }
                }
            }
        }
    }
}
