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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Learn 1.0

import "internal"

FocusScope {
    id: root

    property alias color: background.color
    property string section: ""

    QtObject {
        id: prv

        readonly property int sideMargin: 46
    }

    NavigationSection {
        id: navSec
        name: "Learn"
        enabled: root.visible
        order: 3
        onActiveChanged: function(active) {
            if (active) {
                root.forceActiveFocus()
            }
        }
    }

    onSectionChanged: {
        if (!Boolean(root.section)) {
            return
        }

        tabBar.selectPage(root.section)
    }

    LearnPageModel {
        id: pageModel
    }

    Component.onCompleted: {
        pageModel.load()
    }

    Rectangle {
        id: background
        anchors.fill: parent
        color: ui.theme.backgroundSecondaryColor
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
            enabled: topLayout.enabled && topLayout.visible
            section: navSec
            order: 1
            accessible.name: qsTrc("learn", "Learn")
        }

        StyledTextLabel {
            id: learnLabel
            Layout.fillWidth: true

            text: qsTrc("learn", "Learn")
            font: ui.theme.titleBoldFont
            horizontalAlignment: Text.AlignLeft
        }

        SearchField {
            id: searchField

            Layout.preferredWidth: 220

            navigation.name: "LearnSearch"
            navigation.panel: navSearchPanel
            navigation.order: 1

            visible: tabBar.currentIndex !== 1 // Not visible for Classes tab

            onSearchTextChanged: {
                pageModel.setSearchText(searchText)
            }
        }
    }

    StyledTabBar {
        id: tabBar

        anchors.top: topLayout.bottom
        anchors.topMargin: prv.sideMargin
        anchors.left: parent.left
        anchors.leftMargin: prv.sideMargin
        anchors.right: parent.right
        anchors.rightMargin: prv.sideMargin

        function pageIndex(pageName) {
            switch (pageName) {
            case "get-started": return 0
            //! NOTE: see https://github.com/musescore/MuseScore/issues/14886
            //case "advanced": return 1
            case "classes": return 1
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
            direction: NavigationPanel.Horizontal
            order: 2
            accessible.name: qsTrc("learn", "Learn tab bar")
            enabled: tabBar.enabled && tabBar.visible

            onNavigationEvent: function(event) {
                if (event.type === NavigationEvent.AboutActive) {
                    event.setData("controlName", tabBar.currentItem.navigation.name)
                }
            }
        }

        StyledTabButton {
            text: qsTrc("learn", "Get started")

            navigation.name: "Get started"
            navigation.panel: navTabPanel
            navigation.column: 1
        }

        //! NOTE: see https://github.com/musescore/MuseScore/issues/14886
        /*
        StyledTabButton {
            text: qsTrc("learn", "Advanced")

            navigation.name: "Advanced"
            navigation.panel: navTabPanel
            navigation.column: 2
        }
        */

        StyledTabButton {
            text: qsTrc("learn", "Classes")

            visible: pageModel.classesEnabled()

            navigation.name: "Classes"
            navigation.panel: navTabPanel
            navigation.column: 3
        }
    }

    StackLayout {
        anchors.top: tabBar.bottom
        anchors.topMargin: 28
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        currentIndex: tabBar.currentIndex

        Playlist {
            id: getStartedComp

            playlist: pageModel.startedPlaylist

            navigation.section: navSec
            navigation.order: 3
            navigation.name: "LearnGetStarted"
            navigation.accessible.name: qsTrc("learn", "Get started") + navigation.directionInfo

            backgroundColor: root.color
            sideMargin: prv.sideMargin
        }


        //! NOTE: see https://github.com/musescore/MuseScore/issues/14886
        /*
        Playlist {
            id: advancedComp

            playlist: pageModel.advancedPlaylist

            navigation.section: navSec
            navigation.order: 4
            navigation.name: "LearnAdvanced"
            navigation.accessible.name: qsTrc("learn", "Advanced") + navigation.directionInfo

            backgroundColor: root.color
            sideMargin: prv.sideMargin
        }
        */

        ClassesPage {
            id: classesComp

            visible: pageModel.classesEnabled()

            property var author: pageModel.classesAuthor()

            authorName: author.name
            authorPosition: author.position
            authorRole: author.role
            authorDescription: author.description
            authorAvatarUrl: author.avatarUrl
            authorOrganizationName: author.organizationName
            authorOrganizationUrl: author.organizationUrl

            backgroundColor: root.color

            navigation.section: navSec
            navigation.order: 5
            navigation.name: "LearnClasses"
            navigation.accessible.name: qsTrc("learn", "Classes") + navigation.directionInfo

            sideMargin: prv.sideMargin
        }
    }
}

