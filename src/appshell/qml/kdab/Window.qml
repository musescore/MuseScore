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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.AppShell 1.0
import MuseScore.Shortcuts 1.0
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

import "./HomePage"
import "./NotationPage"
import "./PublishPage"
import "./DevTools"

ApplicationWindow {
    id: root

    width: 800
    height: 600

    visible: true

    title: qsTrc("appshell", "MuseScore 4")

    property var provider: InteractiveProvider {
        topParent: root

        onRequestedDockPage: {
            pagesStack.currentPageUri = uri
        }
    }

    ListView {
        id: mainToolbar

        anchors.top: parent.top

        width: parent.width
        height: 30

        spacing: 8
        orientation: Qt.Horizontal

        model: pagesStack.allPagesUri

        delegate: FlatButton {
            text: modelData

            onClicked: {
                api.launcher.open(modelData)
            }
        }
    }

    StackLayout {
        id: pagesStack

        anchors.top: mainToolbar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        property string currentPageUri: homePage.uri

        currentIndex: allPagesUri.indexOf(currentPageUri)

        property var allPagesUri: [
            homePage.uri,
            notationPage.uri,
            publishPage.uri,
            devtoolsPage.uri
        ]

        HomePage {
            id: homePage

            uri: "musescore://home"
        }

        NotationPage {
            id: notationPage

            uri: "musescore://notation"
        }

        PublishPage {
            id: publishPage

            uri: "musescore://publish"
        }

        DevToolsPage {
            id: devtoolsPage

            uri: "musescore://devtools"
        }
    }
}
