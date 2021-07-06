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
import MuseScore.Diagnostics 1.0

Rectangle {

    color: ui.theme.backgroundPrimaryColor

    Item {
        id: topPanel
        anchors.left: parent.left
        anchors.right: parent.right
        height: 40

        Dropdown {
            id: selector
            anchors.fill: parent
            anchors.margins: 4

            textRole: "title"
            valueRole: "code"

            currentIndex: 0

            model: [
                { code: "paths", title: "Show paths" }
            ]
        }
    }

    Loader {
        anchors.top: topPanel.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        sourceComponent: {
            switch (selector.currentValue) {
            case "paths": return pathsComp
            }
            return defComp
        }
    }

    Component {
        id: defComp
        Rectangle {
            color: "#123456"
        }
    }

    Component {
        id: pathsComp
        DiagnosticsPaths {}
    }
}
