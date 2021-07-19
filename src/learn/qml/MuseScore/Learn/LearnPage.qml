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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Learn 1.0

import "internal"

FocusScope {
    id: root

    property var color: ui.theme.backgroundSecondaryColor
    property string section: ""

    signal requestActiveFocus()

    QtObject {
        id: prv

        readonly property int sideMargin: 46
    }

    NavigationSection {
        id: navSec
        name: "Learn"
        enabled: root.visible
        order: 3
        onActiveChanged: {
            if (active) {
                root.requestActiveFocus()
            }
        }
    }

    onSectionChanged: {
        if (!Boolean(root.section)) {
            return
        }

        bar.selectPage(root.section)
    }

    LearnPageModel {
        id: pageModel
    }

    Component.onCompleted: {
        pageModel.load()
    }

    Rectangle {
        anchors.fill: parent
        color: root.color
    }

    RowLayout {
        id: topLayout
        anchors.top: parent.top
        anchors.topMargin: prv.sideMargin
        anchors.left: parent.left
        anchors.leftMargin: prv.sideMargin
        anchors.right: parent.right
        anchors.rightMargin: prv.sideMargin

        spacing: 12

        NavigationPanel {
            id: navSearchPanel
            name: "LearnSearch"
            section: navSec
            order: 1
            accessible.name: qsTrc("learn", "Learn")
        }

        StyledTextLabel {
            id: learnLabel

            text: qsTrc("learn", "Learn")
            font: ui.theme.titleBoldFont
            horizontalAlignment: Text.AlignLeft
        }

        Item {
            Layout.preferredWidth: topLayout.width - learnLabel.width - searchField.width - topLayout.spacing * 2
            Layout.fillHeight: true
        }

        SearchField {
            id: searchField

            Layout.preferredWidth: 220

            navigation.name: "LearnSearch"
            navigation.panel: navSearchPanel
            navigation.order: 1
            accessible.name: qsTrc("learn", "Learn search")

            onSearchTextChanged: {
                pageModel.setSearchText(searchText)
            }
        }
    }

    TabBar {
        id: bar

        anchors.top: topLayout.bottom
        anchors.topMargin: 36
        anchors.left: parent.left
        anchors.leftMargin: prv.sideMargin - itemSideMargin

        contentHeight: 32
        spacing: 0

        readonly property int itemSideMargin: 22

        function pageIndex(pageName) {
            switch (pageName) {
            case "get-started": return 0
            case "advanced": return 1
            case "classes": return 2
            }

            return 0
        }

        function selectPage(pageName) {
            currentIndex = pageIndex(pageName)
        }

        NavigationPanel {
            id: navTabPanel
            name: "LearnTabs"
            section: navSec
            order: 2
            accessible.name: qsTrc("learn", "Learn tabs")

            onNavigationEvent: {
                if (event.type === NavigationEvent.AboutActive) {
                    event.setData("controlName", bar.currentItem.navigation.name)
                }
            }
        }

        StyledTabButton {
            text: qsTrc("learn", "Get Started")
            sideMargin: bar.itemSideMargin
            isCurrent: bar.currentIndex === 0
            backgroundColor: root.color

            navigation.name: "Get Started"
            navigation.panel: navTabPanel
            navigation.order: 1
            onNavigationTriggered: bar.currentIndex = 0
        }

        StyledTabButton {
            text: qsTrc("learn", "Advanced")
            sideMargin: bar.itemSideMargin
            isCurrent: bar.currentIndex === 1
            backgroundColor: root.color

            navigation.name: "Advanced"
            navigation.panel: navTabPanel
            navigation.order: 2
            onNavigationTriggered: bar.currentIndex = 1
        }

        StyledTabButton {
            text: qsTrc("learn", "Classes")
            sideMargin: bar.itemSideMargin
            isCurrent: bar.currentIndex === 2
            backgroundColor: root.color

            navigation.name: "Classes"
            navigation.panel: navTabPanel
            navigation.order: 3
            onNavigationTriggered: bar.currentIndex = 2
        }
    }

    StackLayout {
        anchors.top: bar.bottom
        anchors.topMargin: 24
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 24

        currentIndex: bar.currentIndex

        Playlist {
            id: getStartedComp

            playlist: pageModel.startedPlaylist

            navigation.section: navSec
            navigation.order: 3
            navigation.name: "LearnGetStarted"
            navigation.accessible.name: qsTrc("learn", "Get Started") + navigation.directionInfo

            sideMargin: prv.sideMargin

            onRequestOpenVideo: {
                pageModel.openVideo(videoId)
            }

            onRequestActiveFocus: {
                root.requestActiveFocus()
            }
        }

        Playlist {
            id: advancedComp

            playlist: pageModel.advancedPlaylist

            navigation.section: navSec
            navigation.order: 4
            navigation.name: "LearnAdvanced"
            navigation.accessible.name: qsTrc("learn", "Advanced") + navigation.directionInfo

            sideMargin: prv.sideMargin

            onRequestOpenVideo: {
                pageModel.openVideo(videoId)
            }

            onRequestActiveFocus: {
                root.requestActiveFocus()
            }
        }

        ClassesPage {
            id: classesComp

            property var author: pageModel.classesAuthor()

            authorName: author.name
            authorPosition: author.position
            authorRole: author.role
            authorDescription: author.description
            authorAvatarUrl: author.avatarUrl
            authorOrganizationName: author.organizationName

            navigation.section: navSec
            navigation.order: 4
            navigation.name: "LearnClasses"
            navigation.accessible.name: qsTrc("learn", "Classes") + navigation.directionInfo

            sideMargin: prv.sideMargin

            onRequestOpenOrganizationUrl: {
                pageModel.openUrl(author.organizationUrl)
            }
        }
    }
}

