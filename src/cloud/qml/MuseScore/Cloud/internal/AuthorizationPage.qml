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

import MuseScore.UiComponents 1.0
import MuseScore.Cloud 1.0

FocusScope {
    id: root

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

    StyledTextLabel {
        id: pageTitle

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: privateProperties.sideMargin
        anchors.leftMargin: privateProperties.sideMargin

        text: qsTrc("scores", "Account")

        font: ui.theme.titleBoldFont
    }

    Image {
        id: logo

        anchors.top: pageTitle.bottom
        anchors.topMargin: 44

        height: 240
        width: parent.width

        visible: root.height > 600

        source: "qrc:/qml/MuseScore/Cloud/resources/mu_logo_background.jpeg"

        Image {
            anchors.centerIn: parent
            source: "qrc:/qml/MuseScore/Cloud/resources/mu_logo.svg"
        }
    }

    AccountBenefitsDescription {
        anchors.top: logo.visible ? logo.bottom : pageTitle.bottom
        anchors.topMargin: 66
        anchors.left: parent.left
        anchors.leftMargin: privateProperties.sideMargin
    }

    Rectangle {
        anchors.bottom: parent.bottom

        height: 114
        width: parent.width

        color: ui.theme.popupBackgroundColor

        RowLayout {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: privateProperties.sideMargin
            anchors.right: parent.right
            anchors.rightMargin: privateProperties.sideMargin

            spacing: 22

            FlatButton {
                Layout.alignment: Qt.AlignLeft

                width: privateProperties.buttonWidth
                text: qsTrc("cloud", "Learn more")

                onClicked: {
                    // TODO: implement me
                }
            }

            Row {
                Layout.alignment: Qt.AlignRight

                spacing: 22

                FlatButton {
                    width: privateProperties.buttonWidth
                    text: qsTrc("cloud", "Sign in")

                    onClicked: {
                        root.signInRequested()
                    }
                }

                FlatButton {
                    width: privateProperties.buttonWidth
                    text: qsTrc("cloud", "Create new account")

                    accentButton: true

                    onClicked: {
                        root.createAccountRequested()
                    }
                }
            }
        }
    }
}
