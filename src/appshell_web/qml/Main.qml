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
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Muse.Ui 1.0
import Muse.UiComponents 1.0

import MuseScore.AppShell 1.0

AppWindow {
    id: root

    flags: Qt.FramelessWindowHint

    InteractiveProvider {
        id: interactiveProvider
        topParent: root

        onRequestedDockPage: function(uri, params) {
            Qt.callLater(interactiveProvider.onPageOpened)
        }
    }

    AppMenuBar {
        id: appMenuBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    Item {
        id: contentItem
        anchors.top: appMenuBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        StyledTabBar {
            id: bar
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 16

            StyledTabButton {
                text: "Notation"
            }
            StyledTabButton {
                text: "Dev"
            }
        }

        StackLayout {
            anchors.top: bar.bottom
            anchors.topMargin: 8
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            currentIndex: bar.currentIndex

            NotationFrame {

            }

            DevFrame {

            }

        }
    }
}
