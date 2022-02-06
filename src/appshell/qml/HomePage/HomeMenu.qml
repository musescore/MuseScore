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
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Cloud 1.0

Item {
    id: root

    property string currentPageName: ""
    property int toggleWidth: 135

    signal selected(string name)

    width: parent.width

    NavigationSection {
        id: navSec
        name: "HomeMenuSection"
        enabled: root.enabled && root.visible
        order: 2
    }

    NavigationPanel {
        id: navPanel
        name: "HomeMenuPanel"
        enabled: root.enabled && root.visible
        section: navSec
        order: 1
        direction: NavigationPanel.Vertical

        accessible.name: qsTrc("appshell", "Home menu") + " " + navPanel.directionInfo
    }

    ColumnLayout {
        id: layout

        property alias accountInfoButton: accountInfo

        anchors.fill: parent

        spacing: 20

        AccountInfoButton {
            id: accountInfo

            Layout.fillWidth: true
            Layout.preferredHeight: 60

            navigation.name: "AccountInfo"
            navigation.panel: navPanel
            navigation.row: 1

            checked: root.currentPageName === "account"

            onToggled: {
                root.selected("account")
            }

            onUserAuthorizedChanged: {
                root.selected("scores")
            }

            function showAvatarOnly() {
                textShowed = ""
            }

            function showNormal() {
                textShowed = title
            }
        }

        RadioButtonGroup {
            id: radioButtonList

            Layout.fillHeight: true
            Layout.fillWidth: true

            orientation: ListView.Vertical
            spacing: 0

            model: [
                { "name": "scores", "title": qsTrc("appshell", "Scores"), "icon": IconCode.MUSIC_NOTES },
                { "name": "add-ons", "title": qsTrc("appshell", "Add-ons"), "icon":  IconCode.PLUS },
                { "name": "audio", "title": qsTrc("appshell", "Audio"), "icon":  IconCode.AUDIO },
                { "name": "learn", "title": qsTrc("appshell", "Learn"), "icon":  IconCode.MORTAR_BOARD }
            ]

            currentIndex: 0

            delegate: PageTabButton {
                id: radioButtonDelegate

                width: parent.width

                navigation.name: title
                navigation.panel: navPanel
                navigation.row: 2 + model.index

                spacing: 30
                leftPadding: spacing

                ButtonGroup.group: radioButtonList.radioButtonGroup
                orientation: Qt.Horizontal
                checked: modelData["name"] === root.currentPageName

                title: modelData["title"]

                iconComponent: StyledIconLabel {
                    iconCode: modelData["icon"]
                }

                onToggled: {
                    radioButtonList.currentIndex = index
                    root.selected(modelData["name"])
                }

                onWidthChanged: {
                    if (width < root.toggleWidth) {
                        textShowed = ""
                    } else {
                        textShowed = title
                    }
                }
            }
        }
    }

    onWidthChanged: {
        if (width < root.toggleWidth) {
            layout.accountInfoButton.showAvatarOnly()
        } else {
            layout.accountInfoButton.showNormal()
        }
    }
}
