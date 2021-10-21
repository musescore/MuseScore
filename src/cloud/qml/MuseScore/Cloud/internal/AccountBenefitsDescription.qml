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

Column {
    id: root

    spacing: 24

    property NavigationPanel navigationPanel: null
    property string activeButtonName: ""

    function readInfo() {
        accessibleInfo.focused = true
    }

    function resetFocusOnInfo() {
        accessibleInfo.focused = false
    }

    AccessibleItem {
        id: accessibleInfo
        accessibleParent: root.navigationPanel.accessible
        visualItem: root
        role: MUAccessible.Information
        name: titleLabel.text + ". " + view.generalInfo + root.activeButtonName
    }

    StyledTextLabel {
        id: titleLabel
        text: qsTrc("cloud", "Get the most out of MuseScore with a free account")
        font: ui.theme.headerBoldFont
    }

    Row {
        spacing: 48

        Image {
            sourceSize: Qt.size(110, 110)
            source: "qrc:/qml/MuseScore/Cloud/resources/mu_logo.svg"
        }

        ListView {
            id: view

            anchors.verticalCenter: parent.verticalCenter

            height: contentHeight
            width: contentWidth

            spacing: 6

            property string generalInfo: {
                var result = ""
                for (var i = 0; i < count; i++) {
                    result += model[i] + ". "
                }

                return result
            }

            model: [
                qsTrc("cloud", "Enjoy free cloud storage for your scores"),
                qsTrc("cloud", "Get social by following other creators, and liking and commenting on your favourite works"),
                qsTrc("cloud", "Create a portfolio for your music and share it with millions of users worldwide"),
                qsTrc("cloud", "Upload high quality audio for superb playback")
            ]

            delegate: Row {
                spacing: 16

                Rectangle {
                    width: 9
                    height: width
                    color: ui.theme.accentColor
                    radius: width / 2
                    anchors.verticalCenter: parent.verticalCenter
                }

                StyledTextLabel {
                    text: modelData
                    font: ui.theme.largeBodyFont
                }
            }
        }
    }
}
